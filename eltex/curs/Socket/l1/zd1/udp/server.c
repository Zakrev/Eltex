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

#define MSG struct mMessage
#define MSG_SIZE sizeof(MSG)

#define SERVER_PORT 1126
#define SERVER_IP "192.168.2.34"

MSG {
        char data[32];
        char data_2[32];
};

int main() {
        int sct_d;
        struct sockaddr_in addr;
        socklen_t addr_len;
        struct sockaddr_in from;
        socklen_t from_addr_len;
        MSG msg;
        
        sct_d = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sct_d <= 0){
                perror("Error open socket");
                exit(1);
        }
        addr.sin_family = AF_INET;
        addr.sin_port = SERVER_PORT;
        if( inet_aton(SERVER_IP, &addr.sin_addr) < 0 ){
                printf("Error ip addres "SERVER_IP"\n");
                close(sct_d);
                exit(1);
        }
        addr_len = sizeof(struct sockaddr_in);
        if(bind(sct_d, (struct sockaddr *) &addr, addr_len) != 0){
                perror("Error bind socket");
                close(sct_d);
                exit(1);
        }
        from_addr_len = sizeof(struct sockaddr_in);
        if(recvfrom(sct_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &from, &from_addr_len) < 0){
                perror("Error recv");
        } else {
                printf("%s (%s)(%d): %s\n", msg.data_2, inet_ntoa(from.sin_addr), from.sin_port, msg.data);
                sprintf(msg.data, "Hello Client!");
                sprintf(msg.data_2, "From Server");
                if(sendto(sct_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &from, from_addr_len) < 0){
                        perror("Error send");
                }
        }
        
        close(sct_d);            
        return 0;
}
