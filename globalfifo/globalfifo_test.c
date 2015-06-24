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

void input_handler(int signum)
{
    printf("receive a signal from globalfifo, signalnum:%d\n", signum);
}

main()
{
    int fd, oflags;

    fd = open("/dev/globalfifo", O_RDWR, S_IRUSR | S_IWUSR);
    if(fd != -1){
        /* 启动信号驱动机制 */
        signal(SIGIO, input_handler);
        fcntl(fd, F_SETOWN, getpid());
        oflags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, oflags | FASYNC);
        while(1)
            sleep(1000);
    } else {
        printf("device open failure\n");
    }
}
