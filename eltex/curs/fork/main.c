#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
        pid_t pid;
        
        pid = fork();
        
        if(pid == 0){
                execl("/bin/ls", "ls", "-l", (char*)NULL);
        } else {
                wait(0);
        }
        
        return 0;
}