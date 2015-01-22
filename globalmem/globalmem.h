/*************************************************************************
	> File Name: charmodule.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月17日 星期六 18时53分10秒
 ************************************************************************/

#ifndef _CHARMODULE_H
#define _CHARMODULE_H
#endif

#define GLOBALMEM_SIZE 0x1000   // 全局内存最大4K字节
#define MEM_CLEAR 0x1           // 清零全局内存
#define GLOBALMEM_MAJOR 250     // 预设的globalmem的主设备号

// globalmem设备结构体
struct globalmem_dev{
    struct cdev cdev;   //cdev struct which the kernel has define
    unsigned char mem[GLOBALMEM_SIZE];  //globalmem memory
};
