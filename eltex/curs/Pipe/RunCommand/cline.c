#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "rc.h"

int main()
{
        int wfd;
        int readed;
        char buff[256];
        
        wfd = open(FNAME, O_WRONLY);
        if(wfd <= 0){
                perror("Error open output");
                return 1;
        }
        while(1){
                readed = read(0, buff, 256);
                if(readed > 0){
                        write(wfd, buff, readed);
                        write(1, buff, readed);
                }
        }

        return 0;
}