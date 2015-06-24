/*************************************************************************
	> File Name: ex9-2.c
	> Author: 
	> Mail: 
	> Created Time: 2015年06月16日 星期二 21时44分34秒
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<signal.h>
#include<unistd.h>

#define MAX_LEN 100

void input_handler(int num)
{
    char data[MAX_LEN];
    int len;

    /* 读取并输出STDIN_FILENO上的输入 */
    len = read(STDIN_FILENO, &data, MAX_LEN);
    data[len] = 0;
    printf("input available:%s\n", data);
}

main()
{
    int oflags;

    /* 启动信号驱动机制 */
    signal(SIGIO, input_handler);
    fcntl(STDIN_FILENO, F_SETOWN, getpid());
    oflags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oflags | FASYNC);

    /* 最后进入一个死循环，仅为保持进程不终止，如果程序中
     * 没有这个死循环会立即执行完毕 */
    while(1);
}
