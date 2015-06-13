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
#include <linux/semaphore.h>

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
        return 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    if (down_interruptible(&dev->sem))  /*获得信号量*/
        return -ERESTARTSYS;

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
    up(&dev->sem);  /*释放信号量*/
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

    if (down_interruptible(&dev->sem))  /*获得信号量*/
        return -ERESTARTSYS;

    // 用户空间->内核空间 
    if (copy_from_user(dev->mem + p, buf, count))
        ret = -EFAULT;
    else
    {
        *ppos += count;
        ret = count;
        printk(KERN_INFO "writen %d bytes(s) from %d\n", count, p);
    }
    up(&dev->sem);  /*释放信号量*/
    return ret;
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret;
    switch (orig) {
        case 0:     /*从文件开头开始偏移*/
        if (offset < 0) {
            ret = -EINVAL;
            break;
        }
        if ((unsigned int) offset > GLOBALMEM_SIZE){
            ret = -EINVAL;
            break;
        }
        filp->f_pos = (unsigned int) offset;
        ret = filp->f_pos;
        break;

        case 1:     /*从当前位置开始偏移*/
        if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
            ret = -EINVAL;
            break;
        }
        if ((filp->f_pos + offset) < 0){
            ret = -EINVAL;
            break;
        }
        filp->f_pos += offset;
        ret = filp->f_pos;
        break;

        default:
            ret = -EINVAL;
    }
    return ret;
}

long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct globalmem_dev *dev = filp->private_data;
    switch(cmd) {
        case MEM_CLEAR:
        if (down_interruptible(&dev->sem))  /*获得信号量*/
            return -ERESTARTSYS;

        /* 清楚全局内存*/
        memset(dev->mem, 0, GLOBALMEM_SIZE);
        up(&dev->sem);  /*释放信号量*/

        printk(KERN_INFO "globalmem is set to zero\n");
        break;

        default:
        return -EINVAL; /*其他不支持的命令*/
    }
    return 0;
}

static const struct file_operations globalmem_fops = 
{
    .owner = THIS_MODULE,
    .read = globalmem_read,
    .write = globalmem_write,
    .open = globalmem_open,
    .release = globalmem_release,
    .llseek = globalmem_llseek,
    .unlocked_ioctl = globalmem_ioctl,
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
    sema_init(&globalmem_devp->sem, 1);   /*初始化信号量*/
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
