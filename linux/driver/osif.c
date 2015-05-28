/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Linux Driver - OSIF INTC (AXI FIFO)
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Daniel Borkmann, ETH Zürich
 *                 Sebastian Meisner, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  Driver for the AXI-FIFO used to communicate to the
 *                 hardware-threads (successor of fsl_driver).
 *
 * ======================================================================
 */

#include "osif.h"

#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

/*
 * Definition of registers
 *
 *   base_addr - base address as configured in the hardware design
 *   mem_size  - size of the memory region available
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
#define BASE_ADDR  0x75A00000
#define MEM_SIZE   0x10000

#define RECV_REG(mem)        ((mem) + 0x00)
#define SEND_REG(mem)        ((mem) + 0x04)
#define RECV_STATUS_REG(mem) ((mem) + 0x08)
#define SEND_STATUS_REG(mem) ((mem) + 0x0C)

#define RECV_STATUS_EMPTY_MASK 0x80000000
#define RECV_STATUS_FILL_MASK  0xFFFF
#define SEND_STATUS_FULL_MASK  0x80000000
#define SEND_STATUS_REM_MASK   0xFFFF

/*
 * Definition of interrupt numbers (architecture specific)
 *
 *   irq - interrupt number to which the proc control is connected
 */
#if defined(RECONOS_ARCH_zynq)
#define IRQ       90
#elif defined(RECONOS_ARCH_microblaze)
#define IRQ       4
#endif

/*
 * Struct representing the osif device
 *
 *   name      - name to identify the device driver
 *   base_addr - base addr (should always be BASE_ADDR)
 *   mem_size  - memory size (should always be MEM_SIZE)
 *   irq       - irq number (should always be IRQ)
 *
 *   filp - list of instances
 *
 *   mem        - pointer to the io memory
 *   mdev       - misc device data structure
 *   write_lock - spinlock for synchronization
 *   read_lock  - spinlock for synchronization
 */
struct osif_dev {
	char name[25];
	uint32_t base_addr;
	int mem_size;
	int irq;

	struct list_head filps;
	struct msg *cmsg;

	void __iomem *mem;
	struct miscdevice mdev;
	spinlock_t write_lock;
	spinlock_t read_lock;
};

/*
 * Struct representing an open instance of the osif. The user space is
 * allowed to open the osif device multiple times, creating several
 * insatnces multiplexed by the driver.
 *
 *  ctrl_    - settings to arbitrate incoming messages
 *  msgs     - list of buffered messages
 *  msgs_len - number of buffered messages
 *
 *  dev - pointer to the osiv device structure
 *
 *  list - list head
 *  wait - wait queue head
 */
struct osif_filp {
	uint32_t ctrl_mask, ctrl_bits;
	struct list_head msgs;
	int msgs_len;

	struct osif_dev *dev;

	struct list_head list;
	wait_queue_head_t wait;
};

/*
 * Struct, representing a single message received from the osif.
 *
 *   data   - data of the message of size count
 *            the first word is the control word
 *   count  - number of words in payload
 *   rcount - remaining number of words to read
 *
 *   list - list head
 */
struct msg {
	uint32_t *data;
	size_t count;
	size_t ccount;

	struct list_head list;
};

static struct osif_dev osif;


/* == Low level functions ============================================== */

/*
 * Writes a single word to the outgoing osif by busily waiting for not
 * being full.
 *
 *   dev  - pointer to device structure
 *   data - data to write
 */
static inline void write_word(struct osif_dev *dev, uint32_t data) {
	while(ioread32(SEND_STATUS_REG(dev->mem)) & SEND_STATUS_FULL_MASK);
	iowrite32(data, SEND_REG(dev->mem));
}

/*
 * Writes an entire message to the outgoing osif. This message acquires
 * a spin lock to prevent concurrent access.
 *
 *   dev - pointer to device structure
 *   msg - pointer to message structure
 */
static inline void write_msg(struct osif_dev *dev, struct msg *msg) {
	int i;

	spin_lock(&dev->write_lock);

	for (i = 0; i < msg->count; i++) {
		write_word(dev, msg->data[i]);
	}

	spin_unlock(&dev->write_lock);
}

/*
 * Checks if incoming fifo contains words
 *
 *   dev - pointer to device structure
 */
static inline int read_empty(struct osif_dev *dev) {
	return ioread32(RECV_STATUS_REG(dev->mem)) & RECV_STATUS_EMPTY_MASK;
}

/*
 * Reads a single word from the osif without blocking.
 *
 *   dev - pointer to device structure
 */
static inline uint32_t read_word_nb(struct osif_dev *dev) {
	uint32_t data =  ioread32(RECV_REG(dev->mem));

	return data;
}

/*
 * Allocates a message structure from the control word.
 *
 *   dev  - pointer to device structure
 *   ctrl - control word
 */
static inline struct msg *msg_alloc(uint32_t ctrl) {
		struct msg *msg;

		__printk(KERN_DEBUG "[reconos-osif] "
		                    "allocating new message ...\n");

		msg = kmalloc(sizeof(struct msg), GFP_ATOMIC);
		if (!msg) {
			__printk(KERN_WARNING "[reconos-osif] "
			                      "cannot allocate new message\n");

			return 0;
		}

		msg->count = RECONOS_OSIF_LENGTH(ctrl) + 1;
		msg->ccount = 1;
		INIT_LIST_HEAD(&msg->list);

		msg->data = kmalloc(sizeof(uint32_t) * msg->count, GFP_ATOMIC);
		if (!msg->data) {
			__printk(KERN_WARNING "[reconos-osif] "
			                      "cannot allocate new message buffer\n");

			return 0;
		}
		msg->data[0] = ctrl;

		return msg;
}

/*
 * Frees a message.
 *
 *   dev - pointer to device structure
 *   msg - pointer to message structure
 */
static inline void msg_free(struct msg *msg) {
	kfree(msg->data);
	kfree(msg);
}

/*
 * Returns the filp to route the messae to.
 *
 *   dev - pointer to device strucutre
 *   msg - pointer to message structure
 */
static inline struct osif_filp *msg_route(struct osif_dev *dev, struct msg *msg) {
	struct osif_filp *inst;

	list_for_each_entry(inst, &dev->filps, list) {
		if ((msg->data[0] & inst->ctrl_mask) == inst->ctrl_bits) {
			return inst;
		}
	}

	return NULL;
}

/* == File operations ================================================== */

/*
 * Function called when opening the device.
 *
 *    @see kernel documentation
 */
static int osif_open(struct inode *inode, struct file *filp) {
	struct osif_dev *dev;
	struct osif_filp *inst;

	__printk(KERN_DEBUG "[reconos-osif] "
	                    "opening osif\n");

	dev = &osif;

	inst = kmalloc(sizeof(struct osif_filp), GFP_KERNEL);
	if (!inst) {
		__printk(KERN_DEBUG "[reconos-osif] "
		                    "cannot allocate memory for device\n");

		return -1;
	}

	inst->ctrl_mask = 0x00000000;
	inst->ctrl_bits = 0xFFFFFFFF;
	inst->dev = &osif;
	INIT_LIST_HEAD(&inst->msgs);
	inst->msgs_len = 0;
	INIT_LIST_HEAD(&inst->list);
	init_waitqueue_head(&inst->wait);

	list_add(&inst->list, &dev->filps);

	filp->private_data = inst;

	return 0;
}

/*
 * Function called when closing the device.
 *
 *   @see kernel documentation
 */
static int osif_release(struct inode *inode, struct file *filp) {
	struct osif_filp *inst;
	struct msg *msg;

	__printk(KERN_DEBUG "[reconos-osif] "
	                    "closing osif\n");

	inst = (struct osif_filp *)(filp->private_data);

	spin_lock(&inst->dev->read_lock);
	list_del(&inst->list);
	list_for_each_entry(msg, &inst->msgs, list) {
		if (msg->ccount == msg->count) {
			msg_free(msg);
		}
	}
	spin_unlock(&inst->dev->read_lock);

	kfree(inst);

	return 0;
}

/*
 * Function called when reading from device.
 *
 *   @see kernel documentation
 */
static ssize_t osif_read(struct file *filp, char __user *buf,
                         size_t count, loff_t *offp) {
	struct osif_filp *inst;
	struct msg *msg;

	inst = (struct osif_filp *)(filp->private_data);

	__printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
	                    "reading from osif ...\n",
	                    inst->ctrl_mask, inst->ctrl_bits);

	if (wait_event_interruptible(inst->wait, inst->msgs_len > 0) < 0) {
		__printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
		                    "interrupted in waiting, aborting ...\n",
		                    inst->ctrl_mask, inst->ctrl_bits);
		return 0;
	} else {
		__printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
		                    "woken up ...\n",
		                    inst->ctrl_mask, inst->ctrl_bits);
	}

	msg = list_first_entry(&inst->msgs, struct msg, list);
	if (copy_to_user(buf, msg->data, msg->count * sizeof(uint32_t))) {
		__printk(KERN_WARNING "[reconos-osif (0x%08x, 0x%08x)] "
		                      "buffer overflow\n",
		                      inst->ctrl_mask, inst->ctrl_bits);
	}

	__printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
	                    "read message 0x%08x 0x%08x [...] of size %d ...\n",
	                    inst->ctrl_mask, inst->ctrl_bits,
	                    msg->data[0], msg->data[1], msg->count);

	spin_lock(&inst->dev->read_lock);
	list_del(&msg->list);
	inst->msgs_len--;
	spin_unlock(&inst->dev->read_lock);

	msg_free(msg);

	return msg->count * sizeof(uint32_t);
}

/*
 * Function called when writing to device.
 *
 *   @see kernel documentation
 */
static ssize_t osif_write(struct file *filp, const char *buf,
                     size_t count, loff_t *offp) {
	struct osif_filp *inst;
	struct osif_dev *dev;
	struct msg msg;
	char kbuf[count];
	uint32_t *data;

	inst = (struct osif_filp *)(filp->private_data);
	dev = inst->dev;

	if (copy_from_user(kbuf, buf, count)) {
		__printk(KERN_WARNING "[reconos-osif (0x%08x, 0x%08x)] "
		                      "buffer overflow\n",
		                      inst->ctrl_mask, inst->ctrl_bits);
	}
	data = (uint32_t *)kbuf;

	msg.data = data;
	msg.count = count / sizeof(uint32_t);
	msg.ccount = 0;

	__printk(KERN_DEBUG "[reconos-osif (0x%08x, 0x%08x)] "
	                    "writing message 0x%08x 0x%08x [...] of size %d ...\n",
	                    inst->ctrl_mask, inst->ctrl_bits,
	                    msg.data[0], msg.data[1], msg.count);

	write_msg(dev, &msg);

	return count;
}

/*
 * Function called when issuing an ioctl.
 *
 * @see kernel documentation
 */
static long osif_ioctl(struct file *filp, unsigned int cmd,
	                   unsigned long arg) {
	struct osif_filp *inst;
	uint32_t karg;

	inst = (struct osif_filp *)(filp->private_data);
	if (copy_from_user(&karg, (uint32_t __user *)arg, sizeof(uint32_t))) {
		__printk(KERN_WARNING "[reconos-osif] "
		                      "buffer overflow\n");
	}

	switch (cmd) {
		case RECONOS_OSIF_SET_MASK:
			__printk(KERN_DEBUG "[reconos-osif] "
			                    "setting ctrl mask to 0x%x\n", karg);

			inst->ctrl_mask = karg;
			break;

		case RECONOS_OSIF_SET_BITS:
			__printk(KERN_DEBUG "[reconos-osif] "
			                    "setting ctrl bits to 0x%x\n", karg);

			inst->ctrl_bits = karg;
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
static struct file_operations osif_fops = {
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
static irqreturn_t interrupt(int irq, void *ptr) {
	struct osif_dev *dev;
	struct osif_filp *inst;
	unsigned long flags;
	uint32_t data;

	__printk(KERN_DEBUG "[reconos-osif] "
	                    "interrupt handler\n");

	dev = (struct osif_dev *)ptr;

	spin_lock_irqsave(&dev->read_lock, flags);

	while (!read_empty(dev)) {
		if (!dev->cmsg) {
			data = read_word_nb(dev);
			dev->cmsg = msg_alloc(data);
		} else {
			__printk(KERN_DEBUG "[reconos-osif] "
			                    "reading payload %d/%d\n",
			                    dev->cmsg->ccount, dev->cmsg->count - 1);

			dev->cmsg->data[dev->cmsg->ccount++] = read_word_nb(dev);	

			if (dev->cmsg->ccount == dev->cmsg->count) {
				inst = msg_route(dev, dev->cmsg);

				if (!inst) {
					__printk(KERN_DEBUG "[reconos-osif] "
					                    "discarding message since no consumer ...\n");

					msg_free(dev->cmsg);
					dev->cmsg = NULL;
				} else {
					//__printk(KERN_DEBUG "[reconos-osif] "
					//                    "read message 0x%08x 0x%08x ...\n",
					//                    dev->cmsg->data[0], dev->cmsg->data[1]);

					//__printk(KERN_DEBUG "[reconos-osif] "
					//                    "enqueueing message ...\n");

					list_add_tail(&dev->cmsg->list, &inst->msgs);
					inst->msgs_len++;
					wake_up_interruptible(&inst->wait);

					dev->cmsg = NULL;
				}
			}
		}
	}

	spin_unlock_irqrestore(&dev->read_lock, flags);

	return IRQ_HANDLED;
}


/* == Init and exit functions ========================================== */

/*
 * @see header
 */
int osif_init() {
	struct osif_dev *dev;

	__printk(KERN_INFO "[reconos-osif] "
	                   "initializing driver ...\n");

	dev = &osif;

	// initialize device struct
	strncpy(dev->name, "reconos-osif", 25);
	dev->base_addr = BASE_ADDR;
	dev->mem_size = MEM_SIZE;
	dev->irq = IRQ;
	dev->cmsg = NULL;
	INIT_LIST_HEAD(&dev->filps);

	spin_lock_init(&dev->write_lock);
	spin_lock_init(&dev->read_lock);

	// allocating io memory to read registers
	if (!request_mem_region(BASE_ADDR, MEM_SIZE, dev->name)) {
		__printk(KERN_WARNING "[reconos-osif] "
		                      "memory region busy\n");
		goto req_failed;
	}
	dev->mem = ioremap(BASE_ADDR, MEM_SIZE);
	if (!dev->mem) {
		__printk(KERN_WARNING "[reconos-osif] "
		                      "ioremap failed\n");

		goto map_failed;
	}

	// requesting interrupt
	if (request_irq(IRQ, interrupt, 0, dev->name, dev)) {
		__printk(KERN_WARNING "[reconos-osif] "
		                      "can't get irq\n");

		goto irq_failed;
	}
	//disable_irq(IRQ);

	// registering misc device
	dev->mdev.minor = MISC_DYNAMIC_MINOR;
	dev->mdev.fops = &osif_fops;
	dev->mdev.name = dev->name;
	if (misc_register(&dev->mdev) < 0) {
		__printk(KERN_WARNING "[reconos-osif] " 
		                      "error while registering misc-device\n");

		goto reg_failed;
	}

	printk(KERN_INFO "[reconos-osif] "
	                 "driver initialized successfully\n");

	goto out;

reg_failed:
irq_failed:
	iounmap(dev->mem);

map_failed:
	release_mem_region(BASE_ADDR, MEM_SIZE);

req_failed:
	return -1;

out:
	return 0;
}

/*
 * @see header
 */
int osif_exit() {
	struct osif_dev *dev = &osif;

	__printk(KERN_INFO "[reconos-osif] "
	                   "removing driver ...\n");

	osif_reset();

	misc_deregister(&dev->mdev);

	free_irq(IRQ, dev);

	iounmap(dev->mem);
	release_mem_region(BASE_ADDR, MEM_SIZE);

	return 0;
}

/*
 * @see header
 */
void osif_reset() {
	struct osif_dev *dev;

	dev = &osif;

	spin_lock(&dev->read_lock);
	dev->cmsg = NULL;
	spin_unlock(&dev->read_lock);
}