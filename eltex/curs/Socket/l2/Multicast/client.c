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

#define MCAST_PORT 1126
#define MCAST_IP "224.0.0.1"
#define MY_IP "192.168.2.22"

int main() {
        int sock_d;
        struct sockaddr_in ba;
        socklen_t ba_len;
        struct sockaddr_in from;
        socklen_t from_len;
        char msg[MSG_SIZE];
        struct ip_mreqn mreqn;
        
        sock_d = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock_d <= 0){
                perror("Error open socket");
                exit(1);
        }
        
        ba.sin_family = AF_INET;
        ba.sin_port = MCAST_PORT;
        inet_aton(MY_IP, &ba.sin_addr);
        ba_len = sizeof(struct sockaddr_in);       
        bind(sock_d, (struct sockaddr *) &ba, ba_len);        
        /*
                Write to ip_mreqn
        */
        inet_aton(MCAST_IP, &mreqn.imr_multiaddr);
        inet_aton(MY_IP, &mreqn.imr_address);
        mreqn.imr_ifindex = 0;        
        if( setsockopt(sock_d, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreqn, sizeof(struct ip_mreqn)) < 0 ){
                perror("Error setsockopt");
                close(sock_d);
                exit(1);
        }
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
