/*************************************************************************
	> File Name: charmodule.h
	> Author: 
	> Mail: 
	> Created Time: 2015年01月17日 星期六 18时53分10秒
 ************************************************************************/


#ifndef _CHARMODULE_H
#define _CHARMODULE_H
#endif

#define GLOBALFIFO_SIZE 0x1000   // 全局内存最大4K字节
#define FIFO_CLEAR 0x1           // 清零全局内存
#define GLOBALFIFO_MAJOR 250     // 预设的globalfifo的主设备号

// globalfifo设备结构体
struct globalfifo_dev{
    struct cdev cdev;   //cdev struct which the kernel has define
    unsigned int current_len;	//fifo有效数据长度
    unsigned char mem[GLOBALFIFO_SIZE];  //globalfifo memory 
    struct semaphore sem;   //并发控制用的信号量
    wait_queue_head_t r_wait;	//阻塞读用的等待队列头
    wait_queue_head_t w_wait;   //阻塞写用的等待队列头
    struct fasync_struct *async_queue;  //异步结构体指针，用于读
};
