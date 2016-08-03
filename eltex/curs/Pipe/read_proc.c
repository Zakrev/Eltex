#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
/*
        Создает FIFO, читает и выводит на экран
*/

int main()
{
        char buff[256];
        int readed;
        int fd;
        
        
        if( mkfifo("file.fifo", 0777) < 0 ){
                perror("Error on make FIFO");
                return 1;
        }
        fd = open("file.fifo", O_RDONLY);
        if(fd <= 0){
                perror("Error on open FIFO");
                return 2;
        }
        while(1){
                readed = read(fd, buff, 256);
                if(readed > 0){
                        buff[readed] = '\0';
                        printf("%s",  buff);
                        if(strcmp(buff, "close\n") == 0){
                                close(fd);
                                remove("file.fifo");
                                return 0;
                        }
                }
                sleep(1);
        }

        return 0;
}