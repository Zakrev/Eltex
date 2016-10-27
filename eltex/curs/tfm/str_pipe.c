#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARG_LENGHT 5+2
#define ARG_U_SIZE 50

/*
        Функция разбирает входную строку str с позиции pos на команды и параметры
        которые записывает в массив строк arg
        Возвращает позицию символа следующей команды после '|', в строке str.
*/
int GetArgements(char *arg[ARG_LENGHT], char tmp_arg[ARG_LENGHT][ARG_U_SIZE], char *str, int pos)
{
        int i = 0, j;

        for( i = 0; str[pos] != '|' && str[pos] != '\0' && i < ARG_LENGHT; pos++){                
                for(j = 0; j < ARG_U_SIZE && str[pos] != ' ' && str[pos] != '|' && str[pos] != '\0'; j++, pos++){
                        tmp_arg[i][j] = str[pos];
                }
                tmp_arg[i][j] = '\0';
                arg[i] = tmp_arg[i];
                i++;
                if(str[pos] == '\0')
                        break;
        }
        arg[i] = (char*)NULL;
        
        return pos+2;
}

int main()
{
        char str[512] = "ls | cut -c1-5 | cut -c1-4 | cut -c2-3";
        char out[512] = "";
        
        char *arg[ARG_LENGHT];
        char tmp_arg[ARG_LENGHT][ARG_U_SIZE];
        int pipefd_left[2], pipefd_right[2];
        int i, count, pos;
        pid_t pid;
        
        /*
                Подсчет команд
        */
        for(i = 0, count = 0; str[i] != '\0'; i++){
                if(str[i] == '|')
                        count++;
        }
        count += 1;
        
        if(pipe(pipefd_left) < 0){
                perror("Error on create pipe");
                exit(1);
        }
        pos = GetArgements(arg, tmp_arg, str, 0);
        count--;
        pid = fork();
        if(pid == 0){
                close(pipefd_left[0]);
                dup2(pipefd_left[1], 1);
                execvp(arg[0], arg);
                perror(arg[0]);
                exit(1);
        } else {
                close(pipefd_left[1]);
                waitpid(pid, 0, 0);
                while(count > 0){                        
                        if(count <= 0)
                                break;                        
                        if(pipe(pipefd_right) < 0){
                                perror("Error on create pipe");
                                exit(1);
                        }
                        pos = GetArgements(arg, tmp_arg, str, pos);
                        count--;
                        pid = fork();
                        if(pid == 0){
                                dup2(pipefd_right[1], 1);
                                dup2(pipefd_left[0], 0);
                                execvp(arg[0], arg);
                                perror(arg[0]);
                                exit(1);
                        } else {
                                waitpid(pid, 0, 0);
                                close(pipefd_left[0]);
                                close(pipefd_right[1]);                                        
                                if(count <= 0){
                                        goto read_right;      
                                }
                                if(pipe(pipefd_left) < 0){
                                        perror("Error on create pipe");
                                        exit(1);
                                }
                                pos = GetArgements(arg, tmp_arg, str, pos);
                                count--;
                                pid = fork();
                                if(pid == 0){
                                        dup2(pipefd_right[0], 0);
                                        dup2(pipefd_left[1], 1);
                                        execvp(arg[0], arg);
                                        perror(arg[0]);
                                        exit(1);
                                }else{
                                        close(pipefd_right[0]);
                                        close(pipefd_left[1]);
                                }
                        }
                }
                goto read_left;
        }
        
        read_right:
                dup2(pipefd_right[0], 0);
                close(pipefd_right[0]);
        read_left:
                dup2(pipefd_left[0], 0);
                close(pipefd_left[0]);
        
        read(0, out, 512);
        if(out[0] == '\0')
                printf("Output is empty.\n");
        else
                printf("%s", out);
        
        return 0;
}
