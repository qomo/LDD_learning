#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>

#include "example.h"

static int example_major = EXAMPLE_MAJOR;
static int example_minor = 0;

static struct class* example_class = NULL;
static struct example_dev* example_dev = NULL;

module_param(example_major, int, S_IRUGO);
module_param(example_minor, int, S_IRUGO);

static int example_open(struct inode* inode, struct file* filp)
{
	struct example_dev* dev;

	/*
	 * 取得设备结构体example_dev，保存在filp私有数据中，以方便其他函数使用。
	 */
	dev = container_of(inode->i_cdev, struct example_dev, cdev);
	filp->private_data = dev;

	return 0;
}

static int example_release(struct inode* inode, struct file* filp)
{
	return 0;
}

static ssize_t example_read(struct file* filp, char	__user *buf, size_t count, loff_t* f_ops)
{
	struct example_dev* dev = filp->private_data;
	ssize_t retval = 0;

	/*
	 * 加锁
	 */
	if(mutex_lock_interruptible(&(dev->mutex)))
		return -ERESTARTSYS;

	if(count < sizeof(dev->val))
		goto out;

	/*
	 * 读寄存器的值到用户空间
	 */
	if(copy_to_user(buf, &(dev->val), sizeof(dev->val)))
	{
		retval = -EFAULT;
		goto out;
	}

	retval = sizeof(dev->val);

out:
	/*
	 * 解锁
	 */
	 mutex_unlock(&(dev->mutex));
	 return retval;
}

static ssize_t example_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_ops)
{
	struct example_dev* dev = filp->private_data;
	ssize_t retval = 0;

	/*
	 * 加锁
	 */
	if(mutex_lock_interruptible(&(dev->mutex)))
		return -ERESTARTSYS;

	if(count != sizeof(dev->val))
		goto out;

	/*
	 * 从用户空间读取并给寄存器赋值
	 */
	if(copy_from_user(&(dev->val), buf, count))
	{
		retval = -EFAULT;
		goto out;
	}

	retval = sizeof(dev->val);

out:
	/*
	 * 解锁
	 */
	mutex_unlock(&(dev->mutex));
	return retval;
}

/*
 * 设备操作函数集
 */
static struct file_operations example_fops = 
{
	.owner = THIS_MODULE,
	.open = example_open,
	.release = example_release,
	.read = example_read,
	.write = example_write,
};

/*
 * 在同步状态下读取寄存器的值
 */
static ssize_t __example_get_val(struct example_dev* dev, char* buf)
{
	int val = 0;

	if(mutex_lock_interruptible(&(dev->mutex)))
		return -ERESTARTSYS;

	val = dev->val;
	mutex_unlock(&(dev->mutex));

	return snprintf(buf, 30, "%d\n", val);
}

/*
 * 在同步状态下设置寄存器的值
 */
static ssize_t __example_set_val(struct example_dev* dev, const char* buf, size_t count)
{
	int val = 0;

	val = (int)simple_strtol(buf, NULL, 10);

	if(mutex_lock_interruptible(&(dev->mutex)))
		return -ERESTARTSYS;

	dev->val = val;
	mutex_unlock(&(dev->mutex));

	return count;
}

/*
 * 对属性文件的读取操作函数
 */
static ssize_t example_val_show(struct device* dev, struct device_attribute *attr, char *buf)
{
	struct example_dev* hdev = (struct example_dev*)dev_get_drvdata(dev);

	return __example_get_val(hdev, buf);
}

/*
 * 对属性文件的写操作函数。
 */
static ssize_t example_val_store(struct device* dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct example_dev* hdev = (struct example_dev*)dev_get_drvdata(dev);

	return __example_set_val(hdev, buf, count);
}

/*
 * DEVICE_ATTR宏展开后生成的是dev_attr_val。
 * 指定属性名为“val”，对应的读写函数分别是example_val_show和example_val_store。
 */
static DEVICE_ATTR(val, S_IRUGO | S_IWUSR, example_val_show, example_val_store);
// static DEVICE_ATTR(val, S_IRUGO | S_IWUSR, example_val_show, NULL);

/*
 * /proc 节点的读操作函数。
 */
static ssize_t example_proc_read(char* page, char** start, off_t off, int count, int* eof, void* data)
{
	if(off > 0)
	{
		*eof = 1;
		return 0;
	}

	return __example_get_val(example_dev, page);
}

/*
 * /proc 节点的写操作函数。
 */
static ssize_t example_proc_write(struct file* filp, const char __user *buff, unsigned long len, void* data)
{
	int err = 0;
	char* page = NULL;

	if(len > PAGE_SIZE)
	{
		printk(KERN_ALERT "The buff is too large: %lu.\n", len);
		return -EFAULT;
	}

	page = (char*)__get_free_page(GFP_KERNEL);
	if(!page)
	{
		printk(KERN_ALERT "Failed to alloc page.\n");
		return -ENOMEM;
	}

	if(copy_from_user(page, buff, len))
	{
		printk(KERN_ALERT "Failed to copy buff from user.\n");
		err = -EFAULT;
		goto out;
	}

	err = __example_set_val(example_dev, page, len);

out:
	free_page((unsigned long)page);
	return err;
}

/*
 * 创建/proc节点
 */
static void example_create_proc(void)
{
	struct proc_dir_entry* entry;

	entry = create_proc_entry(EXAMPLE_DEVICE_PROC_NAME, 0, NULL);
	if(entry)
	{
		// entry->owner = THIS_MODULE;
		entry->read_proc = example_proc_read;
		entry->write_proc = example_proc_write;
	}
}

/*
 * 删除/proc节点
 */
static void example_remove_proc(void)
{
	remove_proc_entry(EXAMPLE_DEVICE_PROC_NAME, NULL);
}

/*
 * 初始化设备结构体example_dev。
 */
static int __example_setup_dev(struct example_dev* dev)
{
	int retval;

	/*
	 * 取得设备编号
	 */
	dev_t devno = MKDEV(example_major, example_minor);

	/*
	 * 将设备结构体内存空间初始化为0。
	 */
	 memset(dev, 0, sizeof(struct example_dev));

	 /*
	  * 初始化设备结构体的cdev成员，指定owner和操作函数集。
	  */
	 cdev_init(&(dev->cdev), &example_fops);
	 dev->cdev.owner = THIS_MODULE;
	 dev->cdev.ops = &example_fops;

	 /*
	  * 调用cdev_add，通知内核该字符设备的存在。
	  */
	 retval = cdev_add(&(dev->cdev), devno, 1);
	 if(retval)
	 {
	 	return retval;
	 }

	 /*
	  * 初始化信号量
	  */
	 mutex_init(&(dev->mutex));

	 /*
	  * 将寄存器val的值初始化为0
	  */
	 dev->val = 0;

	 return 0;
}


/* 
 * 模块初始化函数。
 */
static int __init example_init(void)
{
	int retval = -1;
	dev_t dev = 0;
	struct device* device = NULL;

	printk(KERN_ALERT "Initializing example device.\n");

	/*
	 * 如果用户指定了主设备号，即example_major不为0，则调用
	 * register_chrdev_region分配指定的设备编号。
	 * 如果用户没有指定主设备号，即example_major为0，则调用
	 * alloc_chrdev_region动态分配设备编号。
	 */
	if (example_major) {
		dev = MKDEV(example_major, example_minor);
		retval = register_chrdev_region(dev, 1, EXAMPLE_DEVICE_NODE_NAME);
	} else {
		retval = alloc_chrdev_region(&dev, example_minor, 1, EXAMPLE_DEVICE_NODE_NAME);
	}
	if (retval < 0) {
		printk(KERN_WARNING "can't get example_major %d\n", example_major);
		goto fail;
	}

	/*
	 * 取得主设备号和次设备号
	 */
	example_major = MAJOR(dev);
	example_minor = MINOR(dev);

	/*
	 * 为设备结构体example_dev动态分配内存空间。
	 */
	example_dev = kmalloc(sizeof(struct example_dev), GFP_KERNEL);
	if(!example_dev)
	{
		retval = -ENOMEM;
		printk(KERN_ALERT "Failed to alloc example_dev.\n");
		goto unregister;
	}

	/*
	 * 调用__example_setup_dev函数对example_dev结构体进行初始化。
	 */
	retval = __example_setup_dev(example_dev);
	if(retval)
	{
		printk(KERN_ALERT "Failed to setup dev: %d.\n", retval);
		goto cleanup;
	}

	/*
	 * 创建类example， class_create函数执行成功后，在/sys/class目录下
	 * 就会出现一个名为example的目录
	 */
	example_class = class_create(THIS_MODULE, EXAMPLE_DEVICE_CLASS_NAME);
	if(IS_ERR(example_class))
	{
		retval = PTR_ERR(example_class);
		printk(KERN_ALERT "Failed to create example class.\n");
		goto destroy_cdev;
	}

	/*
	 * 创建设备，device_create函数执行成功后，
	 * 和/sys/class/example/example目录及相关文件。
	 * 注意device的类型是struct device，代表一个设备。
	 */
	device = device_create(example_class, NULL, dev, "%s", EXAMPLE_DEVICE_FILE_NAME);
	if(IS_ERR(device))
	{
		retval = PTR_ERR(device);
		printk(KERN_ALERT "Failed to create example device.\n");
		goto destroy_class;
	}

	/* 
	 * 创建属性文件，对应的属性操作函数由dev_attr_val指定。
	 */
	retval = device_create_file(device, &dev_attr_val);
	if(retval < 0)
	{
		printk(KERN_ALERT "Failed to create attribute val.");
		goto destroy_device;
	}

	/*
	 * 将example_dev保存在设备私有数据区中。
	 */
	dev_set_drvdata(device, example_dev);

	/*
	 * 创建proc节点。
	 */
	example_create_proc();

	printk(KERN_ALERT "Succedded to initialize example device.\n");
	return 0;

destroy_device:
 	device_destroy(example_class, dev);

destroy_class:
	class_destroy(example_class);

destroy_cdev:
	cdev_del(&(example_dev->cdev));

cleanup:
 	kfree(example_dev);

unregister:
	unregister_chrdev_region(MKDEV(example_major, example_minor), 1);

fail:
	return retval;
}

/*
 * 模块清理函数。
 */
static void __exit example_exit(void)
{
	dev_t dev = MKDEV(example_major, example_minor);

	printk(KERN_ALERT "Destroy example device.\n");

	example_remove_proc();

	if(example_class)
	{
		device_destroy(example_class, MKDEV(example_major, example_minor));
		class_destroy(example_class);
	}

	if(example_dev)
	{
		cdev_del(&(example_dev->cdev));
		kfree(example_dev);
	}

	unregister_chrdev_region(dev, 1);
}

MODULE_LICENSE("GPL");

module_init(example_init);
module_exit(example_exit);

