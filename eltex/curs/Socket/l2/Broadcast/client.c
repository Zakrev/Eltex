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
        struct sockaddr_in ba;
        socklen_t ba_len;
        struct sockaddr_in from;
        socklen_t from_len;
        char msg[MSG_SIZE];
        
        sock_d = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock_d <= 0){
                perror("Error open socket");
                exit(1);
        }
        
        ba.sin_family = AF_INET;
        ba.sin_port = BCAST_PORT;
        inet_aton(BCAST_IP, &ba.sin_addr);
        ba_len = sizeof(struct sockaddr_in);       
        bind(sock_d, (struct sockaddr *) &ba, ba_len);        
                
        from_len = sizeof(struct sockaddr_in);
        if(recvfrom(sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &from, &from_len) < 0){
                perror("Error recvfrom");
                close(sock_d);
                exit(1);
        }
        printf("%s(%d): %s\n", inet_ntoa(from.sin_addr), from.sin_port, msg);
                    
        close(sock_d);
        return 0;
}
