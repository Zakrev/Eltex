#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "rc.h"

int main()
{
        int rfd, wfd;
        pid_t pid;
        
        if( mkfifo(INPUTF, 0777) < 0 ){
                perror("Error create "INPUTF);
                return 1;
        }
        if( mkfifo(OUTPUTF, 0777) < 0 ){
                perror("Error create "OUTPUTF);
                remove(INPUTF);
                return 2;
        }
        wfd = open(OUTPUTF, O_WRONLY);
        if(wfd <= 0){
                perror("Error open "OUTPUTF);
                remove(INPUTF);
                remove(OUTPUTF);
                return 3;
        }
        rfd = open(INPUTF, O_RDONLY);
        if(rfd <= 0){
                perror("Error open "INPUTF);
                remove(INPUTF);
                remove(OUTPUTF);
                return 4;
        }
        //dup2(rfd, 0);
        //dup2(wfd, 1);
        close(rfd);
        close(wfd);
        remove(INPUTF);
        remove(OUTPUTF);
        
        return 0;
}