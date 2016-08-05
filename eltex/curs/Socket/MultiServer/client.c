#include "inet.h"

#define SADDR struct sockaddr_in
#define SADDR_SIZE sizeof(SADDR)

int main()
{
        int sock_d;
        socklen_t sl;
        SADDR server;
        MSG msg;

	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();
	fork();

        sock_d = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sock_d <= 0){
                perror("Error create socket");
                exit(1);
        }
        
        server.sin_family = AF_INET;
        server.sin_port = htons(SERVER_PORT);
        inet_aton(SERVER_IP, &server.sin_addr);
        sl = SADDR_SIZE;

        if(sendto(sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &server, sl) < 0){
                perror("Error send");
        } else {
                if(recvfrom(sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &server, &sl) < 0){
                        perror("Error recv");
                } else {
                        printf("%s(%d): %s", inet_ntoa(server.sin_addr), htons(server.sin_port), msg.data_str);
			sprintf(msg.data_str, "Thanks!");
			if(sendto(sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &server, sl) < 0)
			        perror("Error send Thanks");
			recvfrom(sock_d, &msg, MSG_SIZE, 0, (struct sockaddr *) &server, &sl);
			printf("%s(%d): %s", inet_ntoa(server.sin_addr), htons(server.sin_port), msg.data_str);
                }
        }
        close(sock_d);
        return 0;
}
