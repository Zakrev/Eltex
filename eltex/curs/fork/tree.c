#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
        pid_t pid;
        
        printf("Pid\tPPid\n");
        printf("%d\t%d\n", getpid(), getppid());
        pid = fork();
        if(pid == 0){
                printf("%d\t%d\n", getpid(), getppid());
                pid = fork();
                if(pid == 0){
                        printf("%d\t%d\n", getpid(), getppid());
                } else {
                        pid = fork();
                        if(pid == 0){
                                printf("%d\t%d\n", getpid(), getppid());
                        } else {
                                wait(0);
                                wait(0);
                        }
                }
        } else {
                pid = fork();
                if(pid == 0){
                        printf("%d\t%d\n", getpid(), getppid());
                        pid = fork();
                        if(pid == 0){
                                printf("%d\t%d\n", getpid(), getppid());
                        } else
                                wait(0);
                } else {
                                wait(0);
                                wait(0);
                }
        }
        
        return 0;
}