#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define LENGHT 5

void job()
{
        char str[512];
        
        scanf("%s", str);
        sprintf(str, "%s_0", str);                                
        printf("%s", str);
}

int main()
{
        char str[256] = "Hello_world!";
        int pipefd_left[2], pipefd_right[2];
        int i;
        pid_t pid;
        
        if( pipe(pipefd_left) < 0)
                exit(0);
        
        write(pipefd_left[1], str, 13);
        close(pipefd_left[1]);
        
        for(i = 0; i < LENGHT; i++){
                if(pipe(pipefd_right) < 0){
                        perror("Error on create pipe");
                        exit(1);
                } else {
                        pid = fork();
                        if(pid == 0){
                                dup2(pipefd_right[1], 1);
                                dup2(pipefd_left[0], 0);
                                job();
                                exit(0);
                        }else{
                                close(pipefd_left[0]);
                                close(pipefd_right[1]);
                                if(pipe(pipefd_left) < 0){
                                        perror("Error on create pipe");
                                        exit(1);
                                }
                                pid = fork();
                                if(pid == 0){
                                        dup2(pipefd_right[0], 0);
                                        dup2(pipefd_left[1], 1);
                                        job();
                                        exit(0);
                                }else{
                                        close(pipefd_right[0]);
                                        close(pipefd_left[1]);
                                }
                        }
                }
        }
        dup2(pipefd_left[0], 0);
        close(pipefd_left[0]); 
        scanf("%s", str);
        printf("%s\n", str);

        return(0);
}
