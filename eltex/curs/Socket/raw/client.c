#include "inet.h"

#include <netinet/udp.h>
#include <netinet/ip.h>

#define SADDR struct sockaddr_in
#define SADDR_SIZE sizeof(SADDR)

#define DEVICE_NAME "wlan0"

#define DGRAM_SIZE sizeof(struct iphdr) + sizeof(struct udphdr) + MSG_SIZE

int main()
{
        int sock_d;
        socklen_t sl;
        SADDR dest;       
        char datagram[DGRAM_SIZE];
        struct udphdr *udph;
        struct iphdr *iph;
        int fl = 1;
        MSG *msg;
        
        sock_d = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if(sock_d <= 0){
                perror("Error create socket");
                exit(1);
        }
        
        memset(datagram, 0, sizeof(datagram));
        msg = (MSG *)(datagram + sizeof(struct udphdr) + sizeof(struct iphdr));
        udph = (struct udphdr *)(datagram + sizeof(struct iphdr));
        iph = (struct iphdr *)datagram;
        
        dest.sin_family = AF_INET;
        dest.sin_port = htons(SERVER_PORT);
        inet_aton(SERVER_IP, &dest.sin_addr);
        sl = SADDR_SIZE;
        
        if(setsockopt(sock_d, IPPROTO_IP, IP_HDRINCL, &fl, sizeof(int)) < 0 ){
                perror("Error set sock options");
                close(sock_d);
                exit(1);
        }
        
        if(setsockopt(sock_d, SOL_SOCKET, SO_BINDTODEVICE, DEVICE_NAME, strlen(DEVICE_NAME) + 1) < 0){
                perror("Error bind to device");
                close(sock_d);
                exit(0);
        }
        
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 0;
        iph->tot_len = htons(DGRAM_SIZE);
        iph->id = htons(2240);
        iph->frag_off = 0;
        iph->ttl = 10;
        iph->protocol = IPPROTO_UDP;
        iph->check = 0;
        iph->saddr = inet_addr("123.123.123.1");
        iph->daddr = dest.sin_addr.s_addr;
        
        udph->source = htons(7777);
        udph->dest = htons(SERVER_PORT);
        udph->len = htons(DGRAM_SIZE - sizeof(struct iphdr));
        udph->check = 0;
        
        if(sendto(sock_d, datagram, DGRAM_SIZE, 0, (struct sockaddr *) &dest, sl) <= 0){
                perror("Error send");
        }
        
        close(sock_d);
        
        return 0;
}
