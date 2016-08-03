#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
/*
        include socket()
*/
#include <sys/types.h>
#include <sys/socket.h>
/*
        include inet_aton()
*/
#include <arpa/inet.h>
#include <netinet/in.h>

#define MSG struct mMessage
#define MSG_SIZE sizeof(MSG)

#define SERVER_PORT 1126
#define SERVER_IP "10.0.2.15"

MSG {
        char data[32];
        char data_2[32];
};

int main() {
        int sct_d;
        MSG msg;
        MSG rmsg;
        struct sockaddr_in addr;
        socklen_t addr_len;
                
        sct_d = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sct_d <= 0){
                perror("Error open socket");
                exit(1);
        }
        
        addr.sin_family = AF_INET;
        addr.sin_port = SERVER_PORT;
        //addr.sin_addr.s_addr = INADDR_ANY;
        if(inet_aton(SERVER_IP, &addr.sin_addr) == 0){
                printf("Error on set ip addres "SERVER_IP);
                close(sct_d);
                exit(1);
        }
        addr_len = sizeof(struct sockaddr_in);
        
        connect(sct_d, (struct sockaddr *) &addr, addr_len);       
        if(server < 0){
                perror("Error connect");
                close(sct_d);
                exit(1);
        }
        printf("Connected to "SERVER_IP"\n");       
        if(recv(sct_d, &rmsg, MSG_SIZE, 0) < 0){
                perror("Error recv");
        } else {
                printf("%s: %s\n", rmsg.data_2, rmsg.data);
                sprintf(msg.data, "Hello Server!");
                sprintf(msg.data_2, "From Client.");
                if(send(sct_d, &msg, MSG_SIZE, 0) < 0)
                        perror("Error send");
        }                
        
        close(sct_d);              
        return 0;
}
