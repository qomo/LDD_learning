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
#include <linux/sched.h>

#include "globalfifo.h"


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
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data;     // 获取设备结构体指针
    DECLARE_WAITQUEUE(wait, current); 	// 定义等待队列

    down(&dev->sem);	// 获得信号量
    add_wait_queue(&dev->r_wait, &wait);  // 进入读等待队列

    // 等待FIFO非空
    while(dev->current_len == 0) {
        if (filp->f_flags & O_NONBLOCK) {
	    ret = -EAGAIN;
  	    goto out;
        }
	__set_current_state(TASK_INTERRUPTIBLE); // 改变进程状态为睡眠
	up(&dev->sem);

 	schedule(); 	// 调度其他进程执行
  	if(signal_pending(current)){ 	// 如果是因为信号唤醒
	    ret = - ERESTARTSYS;
	    goto out2;
	}

	down(&dev->sem);
    }

    // 分析和获取有效的写长度
    if (count > dev->current_len)
        count = dev->current_len;

    // 内核空间->用户空间
    if (copy_to_user(buf, dev->mem, count)){
        ret = -EFAULT;
	goto out;
    } else {
	memcpy(dev->mem, dev->mem+count, dev->current_len - count);  // fifo数据前移
	dev->current_len -= count;  // 有效数据长度减少
        printk(KERN_INFO "read %d bytes(s), current_len:%d\n", count, dev->current_len);
	
	wake_up_interruptible(&dev->w_wait); 	// 唤醒写等待队列

	ret = count;
    }
    out: up(&dev->sem);  /*释放信号量*/
    out2: remove_wait_queue(&dev->w_wait, &wait);  // 移除等待队列
    return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data; 

    DECLARE_WAITQUEUE(wait, current);	// 定义等待队列

    down(&dev->sem);  // 获取信号量
    add_wait_queue(&dev->w_wait, &wait);  // 进入写等待队列头

    // 等待FIFO非满
    while(dev->current_len == GLOBALMEM_SIZE) {
	if(filp->f_flags & O_NONBLOCK) { // 如果是非阻塞访问
	    ret = -EAGAIN;
	    goto out;
	}
	__set_current_state(TASK_INTERRUPTIBLE);  // 改变进程状态为睡眠
	up(&dev->sem);

	schedule();  // 调度其他进程执行
	
	if(signal_pending(current)) {  // 如果是因为信号唤醒
	    ret = -ERESTARTSYS;
	    goto out2;
	}

	down(&dev->sem);  // 获得信号量
    }

    if (count > GLOBALMEM_SIZE - dev->current_len)     //要写的字节数太多
        count = GLOBALMEM_SIZE - dev->current_len;

    // 用户空间->内核空间 
    if (copy_from_user(dev->mem + dev->current_len, buf, count)) {
        ret = -EFAULT;
	goto out;
    } else {
	dev->current_len += count;
        printk(KERN_INFO "writen %d bytes(s), current_len:%d\n", count, dev->current_len);

	wake_up_interruptible(&dev->r_wait);  // 唤醒读等待队列

        ret = count;
    }
    out: up(&dev->sem);  /*释放信号量*/
    out2: remove_wait_queue(&dev->w_wait, &wait);
    set_current_state(TASK_RUNNING);
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
    // 动态申请设备结构体的内存
    globalmem_devp=kmalloc(sizeof(struct globalmem_dev),GFP_KERNEL);
    if(!globalmem_devp) { 	// 申请失败
        result=-ENOMEM;
        goto fail_malloc;
    }

    memset(globalmem_devp,0,sizeof(struct globalmem_dev));

    globalmem_setup_cdev(globalmem_devp,0);
    sema_init(&globalmem_devp->sem, 1);   /*初始化信号量*/

    init_waitqueue_head(&globalmem_devp->r_wait); //初始化读等待队列头
    init_waitqueue_head(&globalmem_devp->w_wait); //初始化写等待队列头

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
