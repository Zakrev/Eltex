#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
        int pipefd[2];
        pid_t pid;
        char buff[256];
        int read_ch = 0;
        int readed;
        
        if(pipe(pipefd) < 0){
                perror("Pipe error");
                return 1;
        }
        pid = fork();
        if(pid == 0){
                close(pipefd[0]);
                readed = 5;
                while(readed-- > 0){
                        sprintf(buff, "Step %d", readed);
                        write(pipefd[1], buff, 6);
                        sleep(1);
                }
        }else{
                close(pipefd[1]);
                while( read_ch < 30 ){
                        readed = read(pipefd[0], buff, 256);
                        read_ch += readed;
                        if(readed > 0){
                                buff[readed] = '\0';
                                printf("%s\n", buff);
                        }
                }
        }

        return 0;
}