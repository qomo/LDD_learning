/*************************************************************************
	> File Name: ex9-1.c
	> Author: 
	> Mail: 
	> Created Time: 2015年06月16日 星期二 21时06分29秒
 ************************************************************************/

#include<stdio.h>
#include<signal.h>
#include<stdlib.h>

void sigterm_handler(int signo)
{
    printf("Have caught sig N.O. %d\n", signo);
    exit(0);
}

int main(void)
{
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
    while(1);

    return 0;
}
