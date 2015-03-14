/*************************************************************************
	> File Name: main.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: 2015年03月14日 星期六 19时07分37秒
 ************************************************************************/

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

#define NUM_CMOS_BANKS 2

/* Per-device (per-bank) structure */
struct cmos_dev {
    unsigned short current_pointer;     /* Current pointer within the bank */
    unsigned int size;                  /* Size of the bank */
    int bank_number;                    /* CMOS bank number */
    struct cdev cdev;                   /* The cdev structure */
    char name[10];                      /* Mutexes, spinlocks, wait queues, ..*/
} *cmos_devp[NUM_CMOS_BANKS];

/* File operations structure. Defined in linux/fs.h */
static struct file_operations cmos_fops = {
    .owner = THIS_MODULE,   /* Owner */
//    .open = cmos_open,      /* Open method */
//    .release = cmos_release, /* Release method */
//    .read = cmos_read,      /* Read method */
//    .write = cmos_write,    /* Write method */
//    .llseek = cmos_llseek,  /* Seek method */
//    .ioctl = cmos_ioctl,    /* Ioctl method */
};

static dev_t cmos_dev_number;   /* Allotted device number */
struct class *cmos_class;       /* Tie with the device model */

#define CMOS_BANK_SIZE      (0xFF*8)
#define DEVICE_NAME         "cmos"
#define CMOS_BANK0_INDEX_PORT   0x70
#define CMOS_BANK0_DATA_PORT    0x71
#define CMOS_BANK1_INDEX_PORT   0x72
#define CMOS_BANK1_DATA_PORT    0x73

unsigned char addrports[NUM_CMOS_BANKS] = {CMOS_BANK0_INDEX_PORT,
                                          CMOS_BANK1_INDEX_PORT,};
unsigned char dataports[NUM_CMOS_BANKS] = {CMOS_BANK0_DATA_PORT,
                                          CMOS_BANK1_DATA_PORT,};

/* 
 * Driver Initialization
 */
int __init cmos_init(void)
{
    int i, ret;

    /* Request dynamic allocation of a device major number */
    if (alloc_chrdev_region(&cmos_dev_number, 0, NUM_CMOS_BANKS, DEVICE_NAME) < 0 ) {
        printk(KERN_DEBUG "Can't register device\n");
        return -1;
    }

    /* Populate sysfs entries */
    cmos_class = class_create(THIS_MODULE, DEVICE_NAME);

    for (i=0; i<NUM_CMOS_BANKS; i++) {
        /* Allocate memory for the per-device structure */
        cmos_devp[i] = kmalloc(sizeof(struct cmos_dev), GFP_KERNEL);
        if (!cmos_devp[i]) {
            printk("Bad Kmalloc\n");
            return -ENOMEM;
        }

        /* Request I/O region */
        sprintf(cmos_devp[i]->name, "cmos%d", i);
        if (!(request_region(addrports[i], 2, cmos_devp[i]->name))) {
            printk("cmos: I/O port 0x%x is not free.\n", addrports[i]);
            return -EIO;
        }

        /* Fill in the bank number to correlate this device
         * with the corresponding CMOS bank */
        cmos_devp[i]->bank_number = i;

        /* Connect the file operations with the cdev */
        cdev_init(&cmos_devp[i]->cdev, &cmos_fops);
        cmos_devp[i]->cdev.owner = THIS_MODULE;

        /* Connect the major/minor number to the cdev */
        ret = cdev_add(&cmos_devp[i]->cdev, (cmos_dev_number + i), 1);
        if (ret) {
            printk("Bad cdev\n");
            return ret;
        }

        /* Send uevents to udev, so it'll create /dev nodes */
        device_create(cmos_class, NULL, MKDEV(MAJOR(cmos_dev_number), i), NULL, "cmos%d", i);
    }

    printk("CMOS Driver Initialized.\n");
    return 0;
}

/* Driver Exit */
void __exit cmos_cleanup(void) 
{
    int i;

    /* Release the major number */
    unregister_chrdev_region((cmos_dev_number), NUM_CMOS_BANKS);

    /* Release I/O region */
    for (i=0; i<NUM_CMOS_BANKS; i++) {
        device_destroy(cmos_class, MKDEV(MAJOR(cmos_dev_number), i));
        release_region(addrports[i], 2);
        cdev_del(&cmos_devp[i]->cdev);
        kfree(cmos_devp[i]);
    }

    /* Destroy cmos_class */
    class_destroy(cmos_class);
    return;
}

module_init(cmos_init);
module_exit(cmos_cleanup);
