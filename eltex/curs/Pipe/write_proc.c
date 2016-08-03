#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
/*
        Открывает FIFO и пишет в него
*/
int GetStrLen(char *str)
{
        int i = 0;
        while(str[i] != '\0')
                i++;
        return i;
}

int main()
{
        int fd;
        char buff[256];
        
        fd = open("file.fifo", O_WRONLY);
        if(fd <= 0){
                perror("Error open FIFO");
                return 2;
        }
        while(1){
                scanf("%s", buff);
                sprintf(buff, "%s\n", buff);
                write(fd, buff, GetStrLen(buff));
                if(strcmp(buff, "close\n") == 0){
                        close(fd);
                        return 0;
                }
        }

        return 0;
}