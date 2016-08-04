#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
        close()
*/
#include <unistd.h>
/*
        socket()
*/
#include <sys/types.h>
#include <sys/socket.h>
/*
        inet_aton()
*/
#include <arpa/inet.h>
#include <netinet/in.h>
/*
        Функции времени
*/
#include <time.h>

#define MSG struct mMessage
#define MSG_SIZE sizeof(MSG)
#define STR_SIZE 128

#define SERVER_PORT 1126
#define SERVER_IP "192.168.43.196"

MSG {
        int sid;
        char data_str[STR_SIZE];
};
