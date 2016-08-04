#include "zd.h"

#define SINF struct mServerInfo
#define MAX_CLIENTS 5
#define MSG_SIZE 32

SINF {
        int client[MAX_CLIENTS];
        int sock_d;
        struct sockaddr_in s_addr;
        socklen_t sa_len;
        pthread_t client_pt[MAX_CLIENTS];
};

int main()
{
                


        return 0;
}

