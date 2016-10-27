#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
        char *prog1[4] = {"ls"};
        char *prog2[4] = {"cut", "-c1-3"};
        char str[256] = "no";
        int pipefd_left[2], pipefd_right[2];
        pid_t prg;
        
        if(pipe(pipefd_left) < 0){
                perror("Pipefd left");
        } else
                if(pipe(pipefd_right) < 0){
                        perror("Pipefd right");
                } else {                     
                        prg = fork();
                        if(prg == 0){
                                dup2(pipefd_right[1], 1);
                                execvp(prog1[0], prog1);
                                perror("Error start");
                        } else {
                                waitpid(prg, 0, 0);
                                close(pipefd_right[1]);
                                prg = fork();
                                if(prg == 0){
                                        dup2(pipefd_left[1], 1);
                                        dup2(pipefd_right[0], 0);
                                        execvp(prog2[0], prog2);
                                        perror("Error start");
                                } else {                                     
                                        waitpid(prg, 0, 0);
                                        close(pipefd_left[1]);
                                        
                                        read(pipefd_left[0], str, 100);
                                        printf("%s\n", str);
                                        close(pipefd_left[0]);
                                        close(pipefd_right[0]);
                                }
                        }
                }
        return(0);
}
