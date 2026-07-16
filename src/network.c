#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include "network.h"
#include "crypto.h"

int create_tcpsock(void){
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }
    return sock;
}

void nittalk_listen(int port) {
    int server_fd = create_tcpsock();
    if (server_fd < 0) return;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr))<0){
        perror("bind");
        close(server_fd);
        return;}
    if (listen(server_fd, 5) < 0){
        perror("listen");
        close(server_fd);
        return;}
    printf("Listening on port %d...\n",port);

    while (1){
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd < 0){
            perror("accept");
            continue;}
        struct packet_header hdr;
        ssize_t recv_len = recv(client_fd, &hdr, sizeof(hdr), 0);
        if (recv_len != sizeof(hdr)) {
            fprintf(stderr, "Incomplete header\n");
            close(client_fd);
            continue;
        }
        if (memcmp(hdr.magic, "NIT\0", 4) != 0) {
            fprintf(stderr, "Invalid magic – dropping connection\n");
            close(client_fd);
            continue;}

        uint32_t payload_size = ntohl(hdr.payload_size);
        if (payload_size == 0 || payload_size > 1024 * 1024 * 10) {
            fprintf(stderr, "Invalid file size\n");
            close(client_fd);
            continue;}

        unsigned char *cipher = malloc(payload_size);
        if (!cipher){
            perror("malloc");
            close(client_fd);
            continue;}

        size_t total = 0;
        while (total<payload_size){
            ssize_t n = recv(client_fd, cipher + total, payload_size - total, 0);
            if (n <= 0){
                fprintf(stderr, "Connection lost during transfer\n");
                free(cipher);
                close(client_fd);
                return;}
            total += n;
        }

        uint64_t shared_key = 0x123456789ABCDEF0ULL;
        unsigned char *plain = malloc(payload_size);
        if (!plain){
            perror("malloc");
            free(cipher);
            close(client_fd);
            continue;}
        xor_decrypt(cipher, plain, payload_size, shared_key);

        char save_path[256];
        snprintf(save_path, sizeof(save_path), "received_%s", hdr.filename);
        FILE *out = fopen(save_path,"wb");
        if (!out){
            perror("fopen");
            free(cipher);
            free(plain);
            close(client_fd);
            continue;}
        fwrite(plain, 1, payload_size, out);
        fclose(out);

        printf("Received and decrypted file: %s\n",save_path);
        free(cipher);
        free(plain);
        close(client_fd);
        break;
    }
    close(server_fd);
}

void nittalk_send(char *ip, char *filepath){
    FILE *in = fopen(filepath, "rb");
    if (!in){
        perror("fopen");
        return;
    }
    fseek(in,0,SEEK_END);
    long filsiz_long = ftell(in);
    fseek(in,0,SEEK_SET);
    if (filsiz_long<=0) {
        fprintf(stderr,"Empty file\n");
        fclose(in);
        return;
    }

    size_t filsiz = (size_t)filsiz_long;

    unsigned char *plain = malloc(filsiz);
    if (!plain) {
        perror("malloc");
        fclose(in);
        return;}
    size_t bytes_read = fread(plain,1,filsiz, in);
    fclose(in);
    if (bytes_read != filsiz){
        fprintf(stderr,"Error reading file\n");
        free(plain);
        return;
    }

    uint64_t shared_key = 0x123456789ABCDEF0ULL;
    unsigned char *cipher = malloc(filsiz);
    if (!cipher){
        perror("malloc");
        free(plain);
        return;}
    xor_encrypt(plain,cipher, filsiz,shared_key);

    int sock = create_tcpsock();
    if (sock < 0) {
        free(plain);
        free(cipher);
        return;}

    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(4443);
    if (inet_pton(AF_INET,ip,&target.sin_addr)<=0) {
        perror("inet_pton");
        close(sock);
        free(plain);
        free(cipher);
        return;}

    if(connect(sock,(struct sockaddr*)&target,sizeof(target))<0){
        perror("connect");
        close(sock);
        free(plain);
        free(cipher);
        return;}

    struct packet_header hdr;
    memcpy(hdr.magic, "NIT\0", 4);
    char *base = strrchr(filepath,'/');
    if(base) base++;
    else base = (char*)filepath;
    strncpy(hdr.filename,base,63);
    hdr.filename[63] = '\0';
    hdr.payload_size = htonl((uint32_t)filsiz);

    if(send(sock,&hdr,sizeof(hdr),0)!=sizeof(hdr)){
        perror("send header");
        close(sock);
        free(plain);
        free(cipher);
        return;}

    size_t sent = 0;
    while (sent<filsiz){
        ssize_t n = send(sock,cipher+sent,filsiz-sent,0);
        if (n<=0){
            perror("send payload");
            break;
        }
        sent += n;
    }
    printf("File sent successfully\n");
    close(sock);
    free(plain);
    free(cipher);
}

void handle_nittalk(char **args,int argc){
    if (argc<2){
        fprintf(stderr,"Usage: nittalk -listen <port> OR nittalk -s <IP> -f <file>\n");
        return;
    }
    if(strcmp(args[1],"-listen")==0 && argc==3){
        int port=atoi(args[2]);
        if (port<= 0 || port>65535) {
            fprintf(stderr,"Invalid port\n");
            return;
        }
        nittalk_listen(port);
    }
    else if (strcmp(args[1],"-s") == 0 && argc == 5 && strcmp(args[3],"-f") == 0){
        nittalk_send(args[2],args[4]);
    }
    else fprintf(stderr,"Invalid nittalk arguments\n");
}
