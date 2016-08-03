#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define MMSG struct mMessage
#define MMSG_SIZE sizeof(struct mMessage)
#define QKEY 2745
#define SERVER_T 1L
#define CLIENT_T 2L

MMSG {
        long type;
        char msg[256];
        char from[256];
};