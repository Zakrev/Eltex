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
        int client_1;
                
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
        
        if(bind(sct_d, (struct sockaddr *) &addr, addr_len) != 0){
                perror("Error bind to socked");
                close(sct_d);
                exit(1);
        }
        
        if(listen(sct_d, 1) != 0){
                perror("Error listen socket");
                close(sct_d);
                exit(1);
        }
        
        client_1 = accept(sct_d, (struct sockaddr *) &addr, &addr_len);       
        sprintf(msg.data, "Hello Client!");
        sprintf(msg.data_2, "From Server.");
        if(send(client_1, &msg, MSG_SIZE, 0) < 0)
                perror("Error send");
        if(recv(client_1, &rmsg, MSG_SIZE, 0) < 0)
                perror("Error recv");        
        else
                printf("%s: %s\n", rmsg.data_2, rmsg.data);

        close(sct_d);              
        return 0;
}
