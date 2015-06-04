/*
 * A globalmem driver as an example of char device drivers
 *
 * The initial developer of the original code is Barry Song
 * <author@linuxdriver.cn>. All Rights Reserved
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "globalmem.h"


MODULE_LICENSE("GPL");

static int globalmem_major = GLOBALMEM_MAJOR;
dev_t devno;
struct globalmem_dev *globalmem_devp;

int globalmem_open(struct inode *inode, struct file *filp)
{
    // 将设备结构体指针赋值给文件私有数据指针
    filp->private_data = globalmem_devp;
    return 0;
}

int globalmem_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data;     // 获取设备结构体指针

    // 分析和获取有效的写长度
    if (p >= GLOBALMEM_SIZE)
        return count ? - ENXIO: 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    // 内核空间->用户空间
    if (copy_to_user(buf, (void*)(dev->mem + p), count))
    {
        ret = -EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;
        printk(KERN_INFO "read %d bytes(s) from %d\n", count, p);
    }
    return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data; 

    if (p >= GLOBALMEM_SIZE)        //要写的偏移量越界
        return count ? - ENXIO: 0;
    if (count > GLOBALMEM_SIZE - p)     //要写的字节数太多
        count = GLOBALMEM_SIZE - p;

    // 用户空间->内核空间 
    if (copy_from_user(dev->mem + p, buf, count))
        ret = -EFAULT;
    else
    {
        *ppos += count;
        ret = count;
        printk(KERN_INFO "writen %d bytes(s) from %d\n", count, p);
    }
    return ret;
}

static const struct file_operations globalmem_fops = 
{
    .owner = THIS_MODULE,
    .read = globalmem_read,
    .write = globalmem_write,
    .open = globalmem_open,
    .release = globalmem_release,
};

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int ret;
    dev_t devno = MKDEV(globalmem_major, index);

    cdev_init(&dev->cdev, &globalmem_fops);
    dev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev->cdev, devno, 1);
    if(ret){
        printk("adding globalmem error");
    }
    printk("Setup globalmem cdev success!");
}

static int globalmem_init(void)
{
    int result;


    if(globalmem_major)
    {
        dev_t devno=MKDEV(globalmem_major,0);
        result=register_chrdev_region(devno,1,"globalmem");
    }
    else
    {
        result=alloc_chrdev_region(&devno,0,1,"globalmem");
        globalmem_major=MAJOR(devno);
    }
    if(result<0)
        return result;
    globalmem_devp=kmalloc(sizeof(struct globalmem_dev),GFP_KERNEL);
    if(!globalmem_devp)
    {
        result=-ENOMEM;
        goto fail_malloc;
    }

    memset(globalmem_devp,0,sizeof(struct globalmem_dev));

    globalmem_setup_cdev(globalmem_devp,0);
    return 0;
fail_malloc:unregister_chrdev_region(devno,1);
    return result;
}

static void globalmem_exit(void)
{
    cdev_del(&globalmem_devp->cdev);
    kfree(globalmem_devp);
    unregister_chrdev_region(MKDEV(globalmem_major,0),1);
}


module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_AUTHOR("Qomo Liao");
MODULE_DESCRIPTION("This is a example !\n");
MODULE_ALIAS("A simple example");
