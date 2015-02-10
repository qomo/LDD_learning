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

## 第四章 调试技术


### NOTE
* `create_proc_read_entry` is a deprecated function
https://lkml.org/lkml/2013/4/11/215


## 第五章 并发和竟态


### NOTE
* 源码使用了与书本不同的加锁方式（mutex和semaphore）
http://www.geeksforgeeks.org/mutex-vs-semaphore/

## 第六章 高级字符驱动程序操作
### 学习资源
* LDD3源码分析之ioctl操作
http://blog.csdn.net/liuhaoyutz/article/details/7386254

### NOTE
* ioctl被unlocked_ioctl取代，ioctl实现函数的参数也有变化
http://unix.stackexchange.com/questions/4711/what-is-the-difference-between-ioctl-unlocked-ioctl-and-compat-ioctl
http://stackoverflow.com/questions/1063564/unlocked-ioctl-vs-normal-ioctl


## 附加1 Android驱动程序开发

* android_driver_example.tgz就是这个驱动的源码
* 参考链接

> http://s1.mi.com/open/index.html?20150210#hezimini
源码与参考链接有一点点不同，主要在锁上

* 调试在真实的nexus 5上进行


