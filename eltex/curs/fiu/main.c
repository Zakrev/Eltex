#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>

int main()
{
        int file, tmp_file;
        int size, i;
        char buff[30];
        char fname[128] = "sample.txt";

        //scanf("%s", fname);
        file = open(fname, O_CREAT | O_RDWR, S_IRWXO | S_IRWXU | S_IRWXG);
        sprintf(fname, "%s.tmp", fname);
        tmp_file = open(fname, O_CREAT | O_RDWR, S_IRWXO | S_IRWXU | S_IRWXG);
        if(file == -1 || tmp_file == -1){
                perror("Error on open file");
                exit(-1);
        }
        size = write(file, buff, sizeof(buff));
        if(size == -1){
                perror("Error on write");
                exit(-1);
        }
        close(file);
        /*file = open("sample.txt", O_RDONLY);
        if(file == -1){
                perror("Error on open file");
                exit(-1);
        }
        size = read(file, buff + 5, 5);
        if(size == -1){
                perror("Error on read");
                exit(-1);
        }
        close(file);

        printf("%s\n", buff);

        file = open("sample.txt", O_RDWR);
        if(file == -1){
                perror("Error on open file");
                exit(-1);
        }
        for(i = 0; i > -10; i--){
                lseek(file, i - 1, SEEK_END);
                read(file, buff - i, 1);
        }
        close(file);

        printf("%s\n", buff);
*/
        return 0;
}