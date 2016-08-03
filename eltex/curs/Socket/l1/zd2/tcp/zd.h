#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MSG struct mMessage
#define MSG_SIZE sizeof(MSG)

#define SERVER_IP "10.0.2.15"
#define SERVER_PORT 1126


