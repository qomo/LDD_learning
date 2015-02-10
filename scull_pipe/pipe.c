/*************************************************************************
    > File Name: pipe.c
    > Author: 
    > Mail: 
    >i Created Time: 2015年02月09日 星期一 17时02分23秒
 ************************************************************************/

#include "scull.h"
#include <linux/cdev.h>
#include <asm/uaccess.h>

struct scull_pipe {
    wait_queue_head_t inq, outq;        /* read and write queues */
    char *buffer, *end;                 /* begin of buf, end of buf */
    int buffersize;                     /* used in pointer arithmetic */
    char *rp, *wp;                      /* where to read, where to write */
    int nreaers, nwriters;              /* number of openings for r/w */
    struct fasync_struct *async_queue;  /* asynchronous readers */
    struct mutex mutex;                 /* mutual exclusion semaphore */
    struct cdev cdev;
}

/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations scull_pipe_fops = {
    .owner =    THIS_MODULE,
//    .llseek =   no_llseek,
    .read =     scull_p_read,
//    .write =    scull_p_write,
//    .poll =     scull_p_poll,
//    .unlocked_ioctl =   scull_ioctl,
//    .open =     scull_p_open,
//    .release =  scull_p_release,
//    .fasync =   scull_p_fasync,
};

/*
 * Data managment: read and write
 */
static ssize_t scull_p_read(struct file *filp, char __user *buf, size_t count, loff_t *f_ops)
{
    struct scull_pipe *dev = filp->private_data;

    if (mutex_lock_interruptible(&dev->mutex))
        return -ERESTARTSYS;

    while (dev->rp == dev-wp) { // nothing to read
        mutex_unlock(&dev->mutex); // release the lock
        if (filp->f_flags & O_NONBLOCK)
            return -EAGAIN;
        PDEBUG("\"%s\" reading: going to sleep\n", current->comm);
        if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
            return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
        /* otherwise loop, but first reacquire the lock */
        if (mutex_lock_interruptible(&dev->mutex))
            return -ERESTARTSYS;
    }
    /* ok, data is there, return something */
    if (dev->wp > dev->rp)
        count = min(count, (size_t)(dev->wp - dev->dev->rp));
    else /* the write pointer has wrapped, return data up to dev->end */
        count = min(count, (size_t)(dev->end - dev->dev->rp));
    if (copy_to_user(buf, dev->rp, count)) {
        mutex_unlock(&dev->mutex);
        return -EFAULT;
    }
    dev->rp += count;
    if (dev->rp == dev->end)
        dev->rp = dev-buffer; /* wrapped */
    mutex_unlock(&dev->mutex);

    /* finally, awake any writers and return */
    wake_up_interruptible(&dev->outq);
    PDEBUG("\"%s\" did read %li bytes\n", current->comm, (long)count);
    return count;
}

/* 
 * Set up a cdev entry.
 */
static void scull_p_setup_cdev(struct scull_pipe *dev, int index)
{
    int err, devno = scull_p_devno + index;
    
    cdev_init(&dev->cdev, &scull_pipe_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    /* Fail gracefully if need be */
    if (err)
        printk(KERN_NOTICE "Error %d adding scullpipe%d", err, index);
}

/*
 * Initialize the pipe devs; return how many we did.
 */
int scull_p_init(dev_t firstdev)
{
    int i, result;

    result = register_chrdev_region(firstdev, scull_p_nr_devs, "scullp");
    if (result < 0) {
        printk(KERN_NOTICE "Unable to get scullp region, error %d\n", result);
        return 0;
    }
    scull_p_devno = firstdev;
    scull_p_devices = kmalloc(scull_p_nr_devs * sizeof(struct scull_pipe), GFP_KERNEL);
    if (scull_p_devices == NULL) {
        unregister_chrdev_region(firstdev, scull_p_nr_devs);
        return 0;
    }
    memset(scull_p_devices, 0, scull_p_nr_devs * sizeof(struct scull_pipe));
    for (i = 0; i < scull_p_nr_devs; i++) {
        init_waitqueue_head(&(scull_p_devices[i].inq));
        init_waitqueue_head(&(scull_p_devices[i].outq));
        mutex_init(&scull_p_devices[i].mutex);
        scull_p_setup_cdev(scull_p_devices + i, i);
    }

#ifdef SCULL_DEBUG
    create_proc_read_entry("scullpipe", 0, NULL, scull_read_p_mem, NULL);
#endif 
    return scull_p_nr_devs;
}

