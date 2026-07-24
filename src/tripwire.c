#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

void run_tripwire(const char *whitelist_ip){
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket (raw)");
        fprintf(stderr, "You must run with sudo\n");
        return;
    }

    printf("Tripwire active. Whitelist IP : %s\n", whitelist_ip);

    unsigned char buffer[65536];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);

    while(1){
        ssize_t len = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr*)&src_addr,&addr_len);
        if(len<0){
            perror("recvfrom");
            break;
        }

        struct iphdr *ip_header = (struct iphdr*)buffer;
        char src_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_header->saddr, src_ip, sizeof(src_ip));

        if (strcmp(src_ip, whitelist_ip)!=0){
            printf("ALERT : Hostile packet from %s\n", src_ip);
        }
    }
    close(sock);
}

int main(int argc, char **argv) {
    if (argc<2){
        fprintf(stderr, "Usage: sudo tripwire <teammate_ip>\n");
        return 1;
    }
    run_tripwire(argv[1]);
    return 0;
}
