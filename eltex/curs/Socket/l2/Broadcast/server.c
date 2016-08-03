#include <stdlib.h>
#include <stdio.h>
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

#define MSG_SIZE 32

#define BCAST_PORT 1126
#define BCAST_IP "192.168.2.255"

int main() {
        int sock_d;
        int fl = 1;
        char msg[MSG_SIZE];
        struct sockaddr_in ba;
        socklen_t ba_len;
        
        sock_d = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock_d <= 0){
                perror("Error open sock");
                exit(1);
        }
        
        ba.sin_family = AF_INET;
        ba.sin_port = BCAST_PORT;
        inet_aton(BCAST_IP, &ba.sin_addr);
        ba_len = sizeof(struct sockaddr_in);
        if( setsockopt(sock_d, SOL_SOCKET, SO_BROADCAST, &fl, sizeof(int)) < 0 ){
                perror("Error setsockopt");
                close(sock_d);
                exit(1);
        }
        sprintf(msg, "Hi Broadcast!");
        if(sendto(sock_d, msg, MSG_SIZE, 0, (struct sockaddr *) &ba, ba_len) < 0)
                perror("Error sendto");
   
        close(sock_d);           
        return 0;
}
