# 这是《Linux Device Drivers》的学习记录

## 先列出一些重要的资源
* Linux Kernel 下载地址
https://www.kernel.org/
* Linux 内核之旅
http://www.kerneltravel.net/
* Linux 设备驱动入门系列
http://www.kerneltravel.net/?page_id=476
* liuhaoyutz的博客
http://blog.csdn.net/liuhaoyutz
* 源码 
https://github.com/duxing2007/ldd3-examples-3.x
* 《精通LINUX设备驱动开发》—— 这本书也很不错，适合入门，部分例子来源这本书
* 《Linux设备驱动开发详解》—— 这本书相比于《LDD3》，似乎跟新一点，内容也清晰，结构合理，例子也更详细完整，适合入门。本项目里关于globalxxx的源码来自这本书。
## 第一章 设备驱动程序简介
* Linux Kernel 编译
http://edsionte.com/techblog/archives/3289

## 第二章 构造和运行模块
### 源码目录
* hello_kernel

### 知识点
* Makefile、Kconfig文件
* module_init、module_exit
* 安装（insmod）、卸载（rmmod）、查询（lsmod）模块
* 查看输出（dmesg）

## 第三章 字符设备驱动程序
### 源码目录
* globalmem
* scull

### 知识点
* 三类设备——**字符设备**、块设备、网络接口
* 设备号、主设备号、次设备号
* 设备号的分配与释放
* 文件操作（open、release、write、read等）
* file结构与inode结构
* 字符设备cdev及其注册

### 学习资源
* Linux 内核之旅——字符设备驱动学习（1）、（2）、（再学习）
http://www.kerneltravel.net/?page_id=476
* globalmem——github、博文
https://github.com/jiangchengbin/drivers/blob/master/globalmem/globalmem.c
http://blog.chinaunix.net/uid-28440799-id-3438698.html
* LDD3源码分析之字符设备驱动程序
http://blog.csdn.net/liuhaoyutz/article/details/7383313
* Android字符设备驱动开发
http://blog.csdn.net/liuhaoyutz/article/details/8500300

### NOTE
* 源码使用了与书本不同的加锁方式（mutex和semaphore）  
http://www.geeksforgeeks.org/mutex-vs-semaphore/
* globalmem这个例子应该来自《Linux设备驱动开发详解》一书（第六章）

## 第四章 调试技术

- `create_proc_read_entry` is a deprecated function
https://lkml.org/lkml/2013/4/11/215
- 一个非常好的proc介绍
http://www.linux.com/learn/linux-career-center/37985-the-kernel-newbie-corner-kernel-debugging-using-proc-qsequenceq-files-part-1
 - proc文件系统介绍
 - 一个最简单的proc例子
 - 内核中的version实例
- 另外你开可以看附加2～4


## 第五章 并发和竟态
scull源码使用的是互斥体  
《Linux设备驱动开发详解》一书中的globalmem使用的是信号量

### NOTE
* 源码使用了与书本不同的加锁方式（mutex和semaphore）  
http://www.geeksforgeeks.org/mutex-vs-semaphore/
* 也可以看看《Linux设备驱动开发详解》 第7章

## 第六章 高级字符驱动程序操作
### ioctl小节
* 源码目录－－/scull_ioctl
* LDD3源码分析之ioctl操作
http://blog.csdn.net/liuhaoyutz/article/details/7386254
* 接口处，ioctl被unlocked_ioctl取代，ioctl实现函数的参数也有变化
http://unix.stackexchange.com/questions/4711/what-is-the-difference-between-ioctl-unlocked-ioctl-and-compat-ioctl
http://stackoverflow.com/questions/1063564/unlocked-ioctl-vs-normal-ioctl

### 阻塞型IO 
- 源码目录－－/scull_pipe
- 源码解读：http://blog.csdn.net/liuhaoyutz/article/details/7395057
 - 文中说源码有bug，但我想应该再仔细考虑。博文中的修改并没有体现出`scull_p_setup_cdev()`里分配的buffer在哪里被清理。而且`scull_p_setup_cdev`是无返回类型的，但里面却有一句`return -ENOMEM;`
- 附加了一个阻塞IO的例子——globalfifo（这个例子源自《Linux设备驱动开发详解》）

### poll和select
- 源码目录－－/scull_poll
- 要理解驱动程序中poll函数的作用和实现，必须先理解用户空间中poll和select函数的用法。
 - http://blog.csdn.net/liuhaoyutz/article/details/7400037
- 用户空间的poll和select系统调用
- 驱动程序poll的实现
- 附加了一个轮询的例子——globalfifo（这个例子源自《Linux设备驱动开发详解》）

## 附加1 Android驱动程序开发

* android_driver_example.tgz就是这个驱动的源码
* 参考链接

> http://blog.csdn.net/liuhaoyutz/article/details/8500300
源码与参考链接有一点点不同，主要在锁上

* 调试在真实的nexus 5上进行

## 附加2 使用/proc文件系统来访问Linux内核的内容

例程文件夹 --- proc/

LDD第四章里有用proc文件系统进行调试

这里是网络上的一个例子，感觉比书里的清晰一点，做个补充

### 不同的是
参考链接里的proc_entry用旧的方式生成，即`create_proc_entry`

这个接口在新内核中已经被移除，得用`proc_create`

read、write方法需要通过`struct file_operations`结构赋予

这与链接中的方法很不一样，这里在之前的globalmem驱动程序的基础上

用`proc_creat`方法添加新的proc文件访问驱动接口

> 参考链接：
http://www.ibm.com/developerworks/cn/linux/l-proc.html

## 附加3 /proc_hz一个最简单的proc例子
> 参考链接：http://www.linux.com/learn/linux-career-center/39972-kernel-debugging-with-proc-qsequenceq-files-part-2-of-3

这个例子做的仅仅是通过proc文件系统将内核的HZ值输出

它使用**最新的`proc_create()`来产生proc文件**，使用了seq_file接口

与LDD3不同的是，它使用`single_open()`打开show函数，而不是通过`seq_open()`打开完整的迭代器操作函数`seq_operations`。
关于seq_file和single_open()、seq_open()，可以参看下面的链接
> 参考链接：http://www.ibm.com/developerworks/cn/linux/l-kerns-usrs2/

## 附加4 /proc_evens－－一个完整的seq_file例子
> 参考链接：http://www.linux.com/learn/linux-career-center/44184-the-kernel-newbie-corner-kernel-debugging-with-proc-qsequenceq-files-part-3

这个例子是个独立的seq_file例子，而不像《LDD3》那样是scull驱动的补充。
所以它更清晰而又完整地讲解了seq_file的很多细节，包括：
- `start()`,`show()`,`next()`,`stop()`
- 《LDD3》没有的`stop()`的内容－－也就是释放资源（因为它的start并没有申请资源）
- What If I Want to Print LOTS of Data?
- 了解到参数`*sfile`, `*v`, `*pos`到意义和使用

做过这个例子再去看《LDD3》的这个小节，会有更清晰的理解。

**需要说明的是**
在最新的内核里(linux 3.13.0)，这个例子有一点小bug，也就是前面提到很多次的`create_proc_entry()`被`proc_create()`取代

## 附加 —— platform设备驱动
这个例子源自《Linux设备驱动开发详解》第12章  
书中源码基于LDD6410平台，需通过在板文件里添加`platform_device`的方式注册platform设备  
其实，这个例子是个虚拟硬件的字符设备驱动（也就是前面的globalfifo），可以在普通的x86平台下加载。  
不同的是，在x86平台下没有找到板文件  
这就需要在驱动源码里手动注册和注销platform设备，通过下面两个函数实现：  
`int platform_device_register(struct platform_device *pdev)`  
`int platform_device_unregister(struct platform_device *pdev)`  
可以参考[这篇混乱的博文](http://blog.csdn.net/ufo714/article/details/8595021)

## 附加 ctags+vim 的使用
没做什么事情，就是生成了ctags标签，便于代码查阅，简单介绍一下安装和使用：  
- 安装：`sudo apt-get install exuberant-ctags`
- 生成标签：`ctags -R .`

> 会在当前目录生成一个tags文件，里面记录着函数和变量等的定义和使用的位置。

- 当前目录下用vim打开你要查阅的源码文件，可以使用下面的几个命令：

|Keyboard command	| Action				    |
|-----------------------|-------------------------------------------|
|**Ctrl-]**		|Jump to the tag underneath the cursor 	    |
|:ts <tag> <RET>	|Search for a particular tag		    |
|:tn			|Go to the next definition for the last tag |
|:tp			|Go to the previous definition for the last tag|
|:ts			|List all of the definitions of the last tag|
|**Ctrl-t**		|Jump back up in the tag stack		    |
