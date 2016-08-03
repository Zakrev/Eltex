#include <stdio.h>
#include <stdlib.h>
#include <error.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>

#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define MFNAME "/mymem"
#define MEMSIZE 1024
#define SEMKEY 2352
#define MMSG struct mMessage
#define MMSG_SIZE sizeof(struct mMessage)

#define CLIENT_POS MMSG_SIZE + 1 + MMSG_SIZE
#define SERVER_POS 0

struct sembuf zd1_lock[2] = {{0,0,0},{0,2,0}};
struct sembuf zd1_unlock = {0,-2,0};
/*
        Сообщения "сервера" лежат в файле от его начала до MMSG_SIZE
        Сообщения "клиента" лежат в файле от MMSG_SIZE + 1 до MMSG_SIZE + 1 + MMSG_SIZE
*/
MMSG {
        char msg[256];
        char from[256];
};
