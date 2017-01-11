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
 *   description:  Driver for the control core to allow access to basic
 *                 settings of the ReconOS system.
 *
 * ======================================================================
 */

#include "reconos/linux/control.h"
#include "reconos/linux/bits.h"
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
 *   num_hwts_reg        - number of hardware threads instanciated
 *   pgd_addr_reg        - physical address of page global directory
 *   page_fault_addr_reg - addres whicch caused a page fault
 *   tlb_hits_reg        - number of tlb hits occured since reset
 *   tlb_misses_reg      - number of tlb misses occured since reset
 *   sys_reset_reg       - write to initiate system reset
 *   ic_reg              - register for interconnect
 *   hwt_reg             - register for hardware threads
 *
 *   ic_sig_bit          - bit for signal in ic register
 *   ic_rst_bit          - bit for reset in ic register
 *   ic_rdy_bit          - bit for reset in ic register
 *   hwt_rst_bit         - bit for reset in hwt register
 *   hwt_sig_bit         - bit for signal in hwt register
 */

#define NUM_HWTS_REG(mem)        ((mem) + 0x000)
#define PGD_ADDR_REG(mem)        ((mem) + 0x004)
#define PAGE_FAULT_ADDR_REG(mem) ((mem) + 0x008)
#define TLB_HITS_REG(mem)        ((mem) + 0x00c)
#define TLB_MISSES_REG(mem)      ((mem) + 0x010)
#define SYS_RESET_REG(mem)       ((mem) + 0x014)
#define IC_REG(mem)              ((mem) + 0x018)
#define HWT_REG(mem,hwt)         ((mem) + 0x100 + (hwt) * 4)

#define IC_SIG_BIT  0x00000000
#define IC_RST_BIT  0x00000001
#define IC_RDY_BIT  0x00000002
#define HWT_RST_BIT 0x00000000
#define HWT_SIG_BIT 0x00000001


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

    wait_queue_head_t wait;
    int page_fault;
    void *page_fault_addr;

    spinlock_t lock;
};

/*
 * Static device structure for control.
 */
static struct dev s_dev =
{
    .name = "reconos-control"
};


/* == File operations ================================================== */

/*
 * Function called when opening the device.
 *
 * When opening the device we just set the private data.
 */
static int control_open(struct inode *inode, struct file *file)
{
    __printk(KERN_DEBUG "[reconos-control] "
                        "opening control\n");

    if (file->private_data)
    {
        __printk(KERN_ERR "[reconos-control] "
                          "file structure already have a private data assigned\n");

        return -EINVAL;
    }

    file->private_data = &s_dev;

    return 0;
}

/*
 * Function called when closing the device.
 *
 * When closing  we just unset the private data.
 */
static int control_release(struct inode *inode, struct file *file)
{
    __printk(KERN_DEBUG "[reconos-control] "
                        "closing control\n");

    if (!file->private_data)
    {
        __printk(KERN_ERR "[reconos-control] "
                          "file structure has no private data assigned\n");

        return -EINVAL;
    }

    file->private_data = 0;

    return 0;
}

/*
 * Function called when issuing an ioctl.
 *
 * The IOCTLs are the way to issue requests.
 */
static long control_ioctl(struct file *file, unsigned int cmd,
                       unsigned long arg)
{
    struct dev *dev;

    if (!file->private_data)
    {
        __printk(KERN_ERR "[reconos-control] "
                          "file structure has no private data assigned\n");

        return -EINVAL;
    }

    dev = (struct dev *)file->private_data;

    switch (cmd)
    {
        case RECONOS_CONTROL_GET_NUM_HWTS:
        {
            int num_hwts;

            num_hwts = (int)ioread32(NUM_HWTS_REG(dev->mem));
            if (copy_to_user((int __user *)arg, &num_hwts, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            return 0;
        }

        case RECONOS_CONTROL_GET_TLB_HITS:
        {
            int tlb_hits;

            tlb_hits = (int)ioread32(TLB_HITS_REG(dev->mem));
            if (copy_to_user((int __user *)arg, &tlb_hits, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            return 0;
        }

        case RECONOS_CONTROL_GET_TLB_MISSES:
        {
            int tlb_misses;

            tlb_misses = (int)ioread32(TLB_MISSES_REG(dev->mem));
            if (copy_to_user((int __user *)arg, &tlb_misses, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            return 0;
        }

        case RECONOS_CONTROL_GET_FAULT_ADDR:
        {
            dev->page_fault = 0;
            enable_irq(dev->irq);

            if (wait_event_interruptible(dev->wait, dev->page_fault) < 0)
            {
                __printk(KERN_DEBUG "[reconos-control] "
                                    "interrupted in waiting, aborting ...\n");

                dev->page_fault = 0;
                disable_irq(dev->irq);

                return -EINVAL;
            }
            else
            {
                __printk(KERN_DEBUG "[reconos-control] "
                                    "page fault ...\n");
            }

            dev->page_fault = 0;
            disable_irq(dev->irq);

            if (copy_to_user((void __user *)arg, &dev->page_fault_addr, sizeof(void *)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            return 0;
        }

        case RECONOS_CONTROL_CLEAR_PAGE_FAULT:
        {
            iowrite32(0, PAGE_FAULT_ADDR_REG(dev->mem));

            return 0;
        }

        case RECONOS_CONTROL_SET_PGD_ADDR:
        {
            iowrite32(virt_to_phys(current->mm->pgd), PGD_ADDR_REG(dev->mem));

            return 0;
        }

        case RECONOS_CONTROL_SYS_RESET:
        {
            spin_lock(&dev->lock);
            iowrite32(0, SYS_RESET_REG(dev->mem));
            spin_unlock(&dev->lock);

            return 0;
        }

        case RECONOS_CONTROL_SET_HWT_RESET:
        {
            int hwt;
            uint32_t reg;

            if (copy_from_user(&hwt, (int __user *)arg, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            spin_lock(&dev->lock);
            reg = ioread32(HWT_REG(dev->mem, hwt));
            SET_BIT(reg, HWT_RST_BIT);
            iowrite32(reg, HWT_REG(dev->mem, hwt));
            spin_unlock(&dev->lock);

            return 0;
        }

        case RECONOS_CONTROL_CLEAR_HWT_RESET:
        {
            int hwt;
            uint32_t reg;

            if (copy_from_user(&hwt, (int __user *)arg, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            spin_lock(&dev->lock);
            reg = ioread32(HWT_REG(dev->mem, hwt));
            CLEAR_BIT(reg, HWT_RST_BIT);
            iowrite32(reg, HWT_REG(dev->mem, hwt));
            spin_unlock(&dev->lock);

            return 0;
        }

        case RECONOS_CONTROL_SET_HWT_SIGNAL:
        {
            int hwt;
            uint32_t reg;

            if (copy_from_user(&hwt, (int __user *)arg, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            spin_lock(&dev->lock);
            reg = ioread32(HWT_REG(dev->mem, hwt));
            SET_BIT(reg, HWT_SIG_BIT);
            iowrite32(reg, HWT_REG(dev->mem, hwt));
            spin_unlock(&dev->lock);

            return 0;
        }

        case RECONOS_CONTROL_CLEAR_HWT_SIGNAL:
        {
            int hwt;
            uint32_t reg;

            if (copy_from_user(&hwt, (int __user *)arg, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            spin_lock(&dev->lock);
            reg = ioread32(HWT_REG(dev->mem, hwt));
            CLEAR_BIT(reg, HWT_SIG_BIT);
            iowrite32(reg, HWT_REG(dev->mem, hwt));
            spin_unlock(&dev->lock);

            return 0;
        }

        case RECONOS_CONTROL_SET_IC_SIG:
        {
            int sig;
            uint32_t reg;

            if (copy_from_user(&sig, (int __user *)arg, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            spin_lock(&dev->lock);
            if (sig)
            {
                SET_BIT(reg, IC_SIG_BIT);
            }
            else
            {
                CLEAR_BIT(reg, IC_SIG_BIT);
            }
            reg = ioread32(IC_REG(dev->mem));

            iowrite32(reg, IC_REG(dev->mem));
            spin_unlock(&dev->lock);

            return 0;
        }

        case RECONOS_CONTROL_SET_IC_RST:
        {
            int sig;
            uint32_t reg;

            if (copy_from_user(&sig, (int __user *)arg, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            spin_lock(&dev->lock);
            reg = ioread32(IC_REG(dev->mem));
            if (sig)
            {
                SET_BIT(reg, IC_RST_BIT);
            }
            else
            {
                CLEAR_BIT(reg, IC_RST_BIT);
            }
            iowrite32(reg, IC_REG(dev->mem));
            spin_unlock(&dev->lock);

            return 0;
        }

        case RECONOS_CONTROL_GET_IC_RDY:
        {
            uint32_t reg;
            int sig;

            reg = ioread32(IC_REG(dev->mem));
            sig = GET_BIT(reg, IC_RDY_BIT);

            if (copy_to_user((int __user *)arg, &sig, sizeof(int)))
            {
                __printk(KERN_ERR "[reconos-control] "
                                  "buffer overflow\n");
                return -ENOMEM;
            }

            return 0;
        }

        default:
            return -EINVAL;
    }
}

/*
 * Struct for file operations to register driver.
 *
 *    @see kernel documentation
 */
static struct file_operations control_fops =
{
    .owner          = THIS_MODULE,
    .open           = control_open,
    .release        = control_release,
    .unlocked_ioctl = control_ioctl,
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

    __printk(KERN_INFO "[reconos-control] "
                       "page fault interrupt\n");

    dev = (struct dev *)ptr;

    dev->page_fault_addr = (void *)ioread32(PAGE_FAULT_ADDR_REG(dev->mem));
    dev->page_fault = 1;
    wake_up_interruptible(&dev->wait);

    return IRQ_HANDLED;
}


/* == Probing and removing ============================================= */

static int control_probe(struct platform_device *pdev)
{
    struct dev *dev;
    struct resource res;

    __printk(KERN_INFO "[reconos-control] "
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

    __printk(KERN_INFO "[reconos-control] "
                       "found memory at 0x%08x with size 0x%x\n",
                       dev->base_addr, dev->mem_size);

    if (!request_mem_region(dev->base_addr, dev->mem_size, dev->name))
    {
        __printk(KERN_ERR "[reconos-control] "
                          "memory region busy\n");
        goto req_failed;
    }
    dev->mem = ioremap(dev->base_addr, dev->mem_size);
    if (!dev->mem)
    {
        __printk(KERN_ERR "[reconos-control] "
                          "ioremap failed\n");
        goto map_failed;
    }

    dev->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (!dev->irq)
    {
        __printk(KERN_ERR "[reconos-control] "
                          "irq could not be determined\n");
        goto irq_failed;
    }
    
    __printk(KERN_INFO "[reconos-control] "
                       "found interrupt %d\n", dev->irq);

    if (request_irq(dev->irq, interrupt, 0, dev->name, dev))
    {
        __printk(KERN_ERR "[reconos-control] "
                          "can't get irq\n");
        goto irq_failed;
    }
    disable_irq(dev->irq);
    
    dev->mdev.minor = MISC_DYNAMIC_MINOR;
    dev->mdev.fops = &control_fops;
    dev->mdev.name = dev->name;
    if (misc_register(&dev->mdev) < 0)
    {
        __printk(KERN_WARNING "[reconos-control] " 
                              "error while registering misc-device\n");
        goto mis_failed;
    }

    dev->page_fault = 0;
    init_waitqueue_head(&dev->wait);
    spin_lock_init(&dev->lock);

    __printk(KERN_INFO "[reconos-control] "
                       "driver initialized successfully\n");

    goto out;

mis_failed:
    iounmap(dev->mem);

irq_failed:
    iounmap(dev->mem);

map_failed:
    release_mem_region(dev->base_addr, dev->mem_size);

req_failed:
    return -1;

out:
    return 0;
}

static int control_remove(struct platform_device *pdev)
{
    struct dev *dev;

    __printk(KERN_INFO "[reconos-control] "
                       "removing driver ...\n");

    dev = &s_dev;

    misc_deregister(&dev->mdev);

    free_irq(dev->irq, dev);

    iounmap(dev->mem);
    release_mem_region(dev->base_addr, dev->mem_size);

    return 0;
}

static struct of_device_id control_of_match[] =
{
    { .compatible = "upb,reconos-control-3.1"},
    {}
};
MODULE_DEVICE_TABLE(of, control_of_match);

static struct platform_driver control_platform_driver =
{
    .driver =
    {
        .name = "reconos-control",
        .of_match_table = control_of_match,
    },
    .probe = control_probe,
    .remove = control_remove,
};
module_platform_driver(control_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christoph Rüthing <christoph@muppetnet.net>");
MODULE_DESCRIPTION("A driver for the ReconOS CONTROL for global system settings.");
