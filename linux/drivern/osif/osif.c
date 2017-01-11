/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Linux Driver - OSIF (AXI FIFO)
 *
 *   project:      ReconOS
 *   author:       Christoph Rüthing <christoph@muppetnet.net>
 *   description:  Driver for the interconnect arbitrating the OSIFs of
 *                 the hardware threads.
 *                 Each hardware thread has an OSIF FIFO assigned to it.
 *                 It uses this interface to communicate to the delegate
 *                 thread and allows access to the different resources.
 *                 All OSIF interfaces of all hardware threads are
 *                 connected to the global interconnect which arbitrates
 *                 them and routes messages to the delegate thread to a
 *                 single FIFO interface to the processor. This driver
 *                 manages access to the single FIFO interface by allow
 *                 read and write access, handling of interrupts and
 *                 routing of messages to appropriate threads, as shown
 *                 in the following figure.
 *
 *                 +--------------------+
 *                 |         CPU        | Each delegate thread opens
 *                 |                    | the device provided by the
 *                 | +------+  +------+ | drier and requests messages
 *                 | | DT 0 |  | DT 1 | | from its associated hardware
 *                 | +------+  +------+ | thread.
 *                 |   |  ^      |  ^   |
 *                 |   v  |      v  |   | The driver module reads the
 *                 | +----------------+ | messages from the single FIFO
 *                 | |   OSIF Driver  | | interface and stores them in
 *                 | +----------------+ | a buffer for the delegates.
 *                 +--------|-^---------+
 *                          | |
 *                          v |           The interconnect arbitrates
 *                 +--------------------+ the different OSIFs of the
 *                 |    Interconnect    | hardware threads and provides
 *                 +--------------------+ a single FIFO interface to the
 *                     | ^        | ^     processor.
 *                     v |        v |
 *                  +-------+  +-------+
 *                  | HWT 0 |  | HWT 1 |
 *                  +-------+  +-------+
 *
 * ======================================================================
 */

#include "reconos/linux/osif.h"
#include "reconos/definitions.h"

#include <linux/module.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>


/* == General definitions ============================================== */

#define __printk(...) printk(__VA_ARGS__)
//#define __printk(...) 

/*
 * Definition of registers
 *
 *   recv_reg        - register for receiving
 *   send_reg        - register for sending
 *   recv_status_reg - status register for incoming fifo
 *   send_status_reg - status register for outgoing fifo
 *
 *   recv_status_empty_mask - mask for empty bit
 *   recv_status_fill_mask  - mask for fill status
 *   send_status_full_mask  - mask for full bit
 *   send_status_rem_mask   - mask for remaining status
 */

#define RECV_REG(mem)        ((mem) + 0x00)
#define SEND_REG(mem)        ((mem) + 0x04)
#define RECV_STATUS_REG(mem) ((mem) + 0x08)
#define SEND_STATUS_REG(mem) ((mem) + 0x0C)

#define RECV_STATUS_EMPTY_MASK 0x80000000
#define RECV_STATUS_FILL_MASK  0x0000ffff
#define SEND_STATUS_FULL_MASK  0x80000000
#define SEND_STATUS_REM_MASK   0x0000ffff


/* == Definition of data structures ==================================== */

/*
 * Struct representing the osif device
 *
 *   name      - name to identify the device
 *   base_addr - base addr (parsed from device tree)
 *   mem_size  - memory size (parsed from device tree)
 *   irq       - irq number (parsed from device tree)
 *
 *   filps - list of instances
 *   msg   - the current message being processed
 *
 *   mem  - pointer to the io memory
 *   mdev - misc device data structure
 * 
 *   write_lock - spinlock for read synchronization
 *   read_lock  - spinlock for write synchronization
 */
struct dev
{
    char name[25];
    uint32_t base_addr;
    int mem_size;
    void __iomem *mem;
    int irq;
    struct miscdevice mdev;

    struct list_head filps;
    spinlock_t filps_lock;
    struct msg *msg;

    spinlock_t write_lock;
    spinlock_t read_lock;
};

/*
 * Struct representing an open instance of the osif. The user space is
 * allowed to open the osif device multiple times, creating several
 * insatnces multiplexed by the driver.
 *
 *  ctrl_mask - settings to arbitrate incoming messages
 *  ctrl_bits - settings to arbitrate incoming messages
 *  msgs      - list of buffered messages
 *  msgs_len  - number of buffered messages
 *
 *  dev - pointer to the osif device structure
 *
 *  list - list head
 *  wait - wait queue head
 */
struct filp
{
    struct dev *dev;

    uint32_t ctrl_mask;
    uint32_t ctrl_bits;
    struct list_head msgs;
    size_t msgs_len;
    spinlock_t msgs_lock;
    wait_queue_head_t wait;

    struct list_head list;
};

/*
 * Struct, representing a single message received from the osif.
 *
 *   data         - data of the message of size count
 *                  the first word is the control word
 *   data_len     - number of words in payload
 *   data_len_cur - current number of words already processed
 *
 *   list - list head to put message into filp->msgs
 */
struct msg
{
    uint32_t *data;
    size_t data_len;
    size_t data_len_cur;

    struct list_head list;
};

/*
 * Static device structure for osif.
 */
static struct dev s_dev =
{
    .name = "reconos-osif"
};


/* == Low level functions ============================================== */

/*
 * Checks if outgoing FIFO is full.
 *
 *   dev - poinmter to device structure
 */
static inline int is_full(struct dev *dev)
{
    return ioread32(SEND_STATUS_REG(dev->mem)) & SEND_STATUS_FULL_MASK;
}

/*
 * Writes a single word to the outgoing FIFO by busily waiting for not
 * being full.
 *
 *   dev  - pointer to device structure
 *   data - data to write
 */
static inline void write_word_blocking(struct dev *dev, uint32_t data)
{
    while(is_full(dev));
    iowrite32(data, SEND_REG(dev->mem));
}

/*
 * Writes an entire message to the outgoing FIFO. This message acquires
 * a spin lock to prevent concurrent access.
 *
 *   dev - pointer to device structure
 *   msg - pointer to message structure
 */
static inline void write_msg_blocking(struct dev *dev, struct msg *msg)
{
    size_t i;

    for (i = 0; i < msg->data_len; ++i)
    {
        write_word_blocking(dev, msg->data[i]);
    }
}

/*
 * Checks if incoming FIFO contains any words.
 *
 *   dev - pointer to device structure
 */
static inline int is_empty(struct dev *dev)
{
    return ioread32(RECV_STATUS_REG(dev->mem)) & RECV_STATUS_EMPTY_MASK;
}

/*
 * Reads a single word from the osif without blocking.
 *
 *   dev - pointer to device structure
 */
static inline uint32_t read_word_blocking(struct dev *dev)
{
    while(is_empty(dev));
    return ioread32(RECV_REG(dev->mem));
}

/*
 * Allocates a message structure from the control word.
 *
 *   ctrl - control word
 */
static inline struct msg *msg_alloc(uint32_t ctrl)
{
    struct msg *msg;

    __printk(KERN_DEBUG "[reconos-osif] "
                        "allocating new message ...\n");

    msg = (struct msg *)kmalloc(sizeof(struct msg), GFP_ATOMIC);
    if (!msg)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "cannot allocate new message\n");

        return 0;
    }

    msg->data_len = RECONOS_OSIF_LENGTH(ctrl) + 1;
    msg->data_len_cur = 1;
    INIT_LIST_HEAD(&msg->list);

    msg->data = (uint32_t *)kmalloc(sizeof(uint32_t) * msg->data_len, GFP_ATOMIC);
    if (!msg->data)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "cannot allocate new message buffer\n");

        return 0;
    }
    msg->data[0] = ctrl;

    return msg;
}

/*
 * Frees a message.
 *
 *   msg - pointer to message structure
 */
static inline void msg_free(struct msg *msg)
{
    kfree(msg->data);
    kfree(msg);
}

/*
 * Returns the filp to route the message to.
 *
 *   dev - pointer to device strucutre
 *   msg - pointer to message structure
 */
static inline struct filp *msg_route(struct dev *dev, struct msg *msg)
{
    struct filp *filp;
    struct filp *ret = 0;

    list_for_each_entry(filp, &dev->filps, list)
    {
        if ((msg->data[0] & filp->ctrl_mask) == filp->ctrl_bits)
        {
            ret = filp;
            break;
        }
    }

    return ret;
}

/*
 * Allocates a new filp.
 *
 * Allocates the required memory and initializes the lists and locks defined
 * in the structure. In case no memory could be allocated, a null pointer is
 * returned.
 */
static inline struct filp *filp_alloc(void)
{
    struct filp *filp;

    __printk(KERN_DEBUG "[reconos-osif] "
                        "allocating new filp ...\n");

    filp = (struct filp *)kmalloc(sizeof(struct filp), GFP_KERNEL);
    if (!filp)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "cannot allocate new filp\n");

        return 0;
    }

    filp->dev = &s_dev;
    filp->ctrl_mask = 0x00000000;
    filp->ctrl_bits = 0xffffffff;
    INIT_LIST_HEAD(&filp->msgs);
    filp->msgs_len = 0;
    spin_lock_init(&filp->msgs_lock);
    init_waitqueue_head(&filp->wait);
    INIT_LIST_HEAD(&filp->list);

    return filp;
}

/*
 * Frees a filp and all messages it cotains.
 *
 * Frees all allocated memory for the filp structure, i.e. the contained
 * messages as well as the structure itself. Therefore, do not access th
 * structure any longer after calling this method, since the pointer passed
 * to this function becomes invalid.
 *
 *   filp - pointer to filp structure
 */
static inline void filp_free(struct filp *filp)
{
    struct msg *msg;
    
    list_for_each_entry(msg, &filp->msgs, list)
    {
        msg_free(msg);
    }

    kfree(filp);
}


/* == File operations ================================================== */

/*
 * Function called when opening the device.
 *
 * When opening a device we need to setup several data structures and
 * initialize the device. For the OSIF we need to allocate a filp structure
 * and initialize its locks and lists. Furthermore, we also need to
 * synchronize with the interrupt handler which might be already active when
 * opening a new device.
 */
static int osif_open(struct inode *inode, struct file *file)
{
    struct filp *filp;
    struct dev *dev;
    unsigned long flags;

    __printk(KERN_DEBUG "[reconos-osif] "
                        "opening osif\n");

    if (file->private_data)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "file structure already have a private data assigned\n");

        return -EINVAL;
    }

    filp = filp_alloc();
    dev = filp->dev;

    if (!filp)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "failed opening osif\n");
        return -ENOMEM;
    }

    spin_lock_irqsave(&dev->filps_lock, flags);
    list_add(&filp->list, &filp->dev->filps);
    spin_unlock_irqrestore(&dev->filps_lock, flags);

    file->private_data = filp;

    return 0;
}

/*
 * Function called when closing the device.
 *
 * When closing the OSIF device we need to deallocate our filp structure we
 * created during opening. Again, we have to synchronize with the interrupt
 * handler since a device might be closed during execution of it.
 */
static int osif_release(struct inode *inode, struct file *file)
{
    struct filp *filp;
    struct dev *dev;
    unsigned long flags;

    __printk(KERN_DEBUG "[reconos-osif] "
                        "closing osif\n");

    if (!file->private_data)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "file structure has no private data assigned\n");

        return -EINVAL;
    }

    filp = (struct filp *)file->private_data;
    dev = filp->dev;

    spin_lock_irqsave(&dev->filps_lock, flags);
    list_del(&filp->list);
    spin_unlock_irqrestore(&dev->filps_lock, flags);

    filp_free(filp);
    file->private_data = 0;

    return 0;
}

/*
 * Function called when reading from device.
 *
 *   @see kernel documentation
 */
static ssize_t osif_read(struct file *file, char __user *buf,
                         size_t count, loff_t *offp)
{
    struct filp *filp;
    struct dev *dev;
    struct msg *msg;
    uint32_t *msg_data;
    size_t msg_data_len_byte;
    unsigned long flags;

    if (!file->private_data)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "file structure has no private data assigned\n");

        return -EINVAL;
    }

    filp = (struct filp *)file->private_data;
    dev = filp->dev;

    __printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
                        "reading from osif ...\n",
                        filp->ctrl_mask, filp->ctrl_bits);

    if (wait_event_interruptible(filp->wait, filp->msgs_len > 0) < 0)
    {
        __printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
                            "interrupted in waiting, aborting ...\n",
                            filp->ctrl_mask, filp->ctrl_bits);
        return 0;
    }
    else
    {
        __printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
                            "woken up ...\n",
                            filp->ctrl_mask, filp->ctrl_bits);
    }

    spin_lock_irqsave(&filp->msgs_lock, flags);
    msg = list_first_entry(&filp->msgs, struct msg, list);
    spin_unlock_irqrestore(&filp->msgs_lock, flags);

    msg_data = msg->data;
    msg_data_len_byte = msg->data_len * sizeof(uint32_t);

    if (copy_to_user(buf, msg_data, msg_data_len_byte))
    {
        __printk(KERN_ERR "[reconos-osif (0x%08x, 0x%08x)] "
                          "buffer overflow\n",
                          filp->ctrl_mask, filp->ctrl_bits);

        return -ENOMEM;
    }

    __printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
                        "read message 0x%08x 0x%08x [...] of size %d ...\n",
                        filp->ctrl_mask, filp->ctrl_bits,
                        msg->data[0], msg->data[1], msg->data_len);

    spin_lock_irqsave(&filp->msgs_lock, flags);
    list_del(&msg->list);
    filp->msgs_len--;
    spin_unlock_irqrestore(&filp->msgs_lock, flags);

    msg_free(msg);

    return msg_data_len_byte;
}

/*
 * Function called when writing to device.
 *
 *   @see kernel documentation
 */
static ssize_t osif_write(struct file *file, const char *buf,
                     size_t count, loff_t *offp)
{
    struct filp *filp;
    struct dev *dev;
    struct msg msg;
    uint32_t msg_data[count];

    if (!file->private_data)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "file structure has no private data assigned\n");

        return -EINVAL;
    }

    filp = (struct filp *)file->private_data;
    dev = filp->dev;

    if (copy_from_user(msg_data, buf, count))
    {
        __printk(KERN_ERR "[reconos-osif (0x%08x, 0x%08x)] "
                          "buffer overflow\n",
                          filp->ctrl_mask, filp->ctrl_bits);

        return -ENOMEM;
    }

    msg.data = msg_data;
    msg.data_len = count / sizeof(uint32_t);
    msg.data_len_cur = 0;

    __printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
                        "writing message 0x%08x 0x%08x [...] of size %d ...\n",
                        filp->ctrl_mask, filp->ctrl_bits,
                        msg.data[0], msg.data[1], msg.data_len);

    spin_lock(&dev->write_lock);
    write_msg_blocking(dev, &msg);
    spin_unlock(&dev->write_lock);

    return count;
}

/*
 * Function called when issuing an ioctl.
 *
 * The IOCTLs for the OSIF allow to set the mask and bits used to filter which
 * messages should be routed to which reader.
 */
static long osif_ioctl(struct file *file, unsigned int cmd,
                       unsigned long arg)
{
    struct filp *filp;
    uint32_t karg;

    if (!file->private_data)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "file structure has no private data assigned\n");

        return -EINVAL;
    }

    filp = (struct filp *)file->private_data;

    if (copy_from_user(&karg, (uint32_t __user *)arg, sizeof(uint32_t)))
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "buffer overflow\n");

        return -ENOMEM;
    }

    switch (cmd)
    {
        case RECONOS_OSIF_SET_MASK:
            __printk(KERN_DEBUG "[reconos-osif] "
                                "setting ctrl mask to 0x%x\n", karg);

            filp->ctrl_mask = karg;
            break;

        case RECONOS_OSIF_SET_BITS:
            __printk(KERN_DEBUG "[reconos-osif] "
                                "setting ctrl bits to 0x%x\n", karg);

            filp->ctrl_bits = karg;
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

/*
 * Struct for file operations to register driver.
 *
 *    @see kernel documentation
 */
static struct file_operations osif_fops =
{
    .owner          = THIS_MODULE,
    .open           = osif_open,
    .release        = osif_release,
    .read           = osif_read,
    .write          = osif_write,
    .unlocked_ioctl = osif_ioctl,
};


/* == Interrupt handling =============================================== */

/*
 * Interrupt handler for handling page faults.
 *
 *   @see kernel documentation
 */
static irqreturn_t interrupt(int irq, void *ptr)
{
    struct dev *dev;
    struct filp *filp;
    unsigned long flags_read_lock, flags_msgs_lock, flags_filps_lock;
    uint32_t word;

    __printk(KERN_DEBUG "[reconos-osif] "
                        "interrupt handler\n");

    dev = (struct dev *)ptr;

    spin_lock_irqsave(&dev->read_lock, flags_read_lock);
    while (!is_empty(dev))
    {
        if (!dev->msg)
        {
            word = read_word_blocking(dev);
            dev->msg = msg_alloc(word);
        }
        else
        {
            __printk(KERN_DEBUG "[reconos-osif] "
                                "reading payload %d/%d\n",
                                dev->msg->data_len_cur, dev->msg->data_len - 1);

            dev->msg->data[dev->msg->data_len_cur++] = read_word_blocking(dev);   

            if (dev->msg->data_len_cur == dev->msg->data_len)
            {
                spin_lock_irqsave(&dev->filps_lock, flags_filps_lock);

                filp = msg_route(dev, dev->msg);

                if (!filp)
                {
                    __printk(KERN_DEBUG "[reconos-osif] "
                                        "discarding message since no consumer ...\n");

                    msg_free(dev->msg);

                    dev->msg = 0;
                }
                else
                {
                    __printk(KERN_DEBUG "[reconos-osif] "
                                        "read message 0x%08x 0x%08x ...\n",
                                        dev->msg->data[0], dev->msg->data[1]);

                    spin_lock_irqsave(&filp->msgs_lock, flags_msgs_lock);
                    list_add_tail(&dev->msg->list, &filp->msgs);
                    filp->msgs_len++;
                    spin_unlock_irqrestore(&filp->msgs_lock, flags_msgs_lock);

                    wake_up_interruptible(&filp->wait);

                    dev->msg = 0;
                }

                spin_unlock_irqrestore(&dev->filps_lock, flags_filps_lock);
            }
        }
    }
    spin_unlock_irqrestore(&dev->read_lock, flags_read_lock);

    return IRQ_HANDLED;
}


/* == Probing and removing ============================================= */

static int osif_probe(struct platform_device *pdev)
{
    struct dev *dev;
    struct resource res;

    __printk(KERN_INFO "[reconos-osif] "
                       "probing driver ...\n");

    dev = &s_dev;

    if (of_address_to_resource(pdev->dev.of_node, 0, &res))
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "address could not be determined\n");
        goto req_failed;
    }
    dev->base_addr = res.start;
    dev->mem_size = res.end - res.start + 1;

    __printk(KERN_INFO "[reconos-osif] "
                       "found memory at 0x%08x with size 0x%x\n",
                       dev->base_addr, dev->mem_size);

    if (!request_mem_region(dev->base_addr, dev->mem_size, dev->name))
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "memory region busy\n");
        goto req_failed;
    }
    dev->mem = ioremap(dev->base_addr, dev->mem_size);
    if (!dev->mem)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "ioremap failed\n");
        goto map_failed;
    }

    dev->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (!dev->irq)
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "irq could not be determined\n");
        goto irq_failed;
    }
    
    __printk(KERN_INFO "[reconos-osif] "
                       "found interrupt %d\n", dev->irq);

    if (request_irq(dev->irq, interrupt, 0, dev->name, dev))
    {
        __printk(KERN_ERR "[reconos-osif] "
                          "can't get irq\n");
        goto irq_failed;
    }

    dev->mdev.minor = MISC_DYNAMIC_MINOR;
    dev->mdev.fops = &osif_fops;
    dev->mdev.name = dev->name;
    if (misc_register(&dev->mdev) < 0)
    {
        __printk(KERN_WARNING "[reconos-osif] " 
                              "error while registering misc-device\n");

        goto mis_failed;
    }

    INIT_LIST_HEAD(&dev->filps);
    spin_lock_init(&dev->filps_lock);
    dev->msg = 0;
    spin_lock_init(&dev->write_lock);
    spin_lock_init(&dev->read_lock);

    __printk(KERN_INFO "[reconos-osif] "
                       "driver initialized successfully\n");

    goto out;

mis_failed:
    free_irq(dev->irq, dev);

irq_failed:
    iounmap(dev->mem);

map_failed:
    release_mem_region(dev->base_addr, dev->mem_size);

req_failed:
    return -1;

out:
    return 0;
}

static int osif_remove(struct platform_device *pdev)
{
    struct dev *dev;

    __printk(KERN_INFO "[reconos-osif] "
                       "removing driver ...\n");

    dev = &s_dev;

    misc_deregister(&dev->mdev);

    free_irq(dev->irq, dev);

    iounmap(dev->mem);
    release_mem_region(dev->base_addr, dev->mem_size);

    return 0;
}

static struct of_device_id osif_of_match[] =
{
    { .compatible = "upb,reconos-osif-3.1"},
    {}
};
MODULE_DEVICE_TABLE(of, osif_of_match);

static struct platform_driver osif_platform_driver =
{
    .driver =
    {
        .name = "reconos-osif",
        .of_match_table = osif_of_match,
    },
    .probe = osif_probe,
    .remove = osif_remove,
};
module_platform_driver(osif_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christoph Rüthing <christoph@muppetnet.net>");
MODULE_DESCRIPTION("A driver for the ReconOS OSIF for communication between software and hardware threads.");
