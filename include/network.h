#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#pragma pack(push, 1)
struct packet_header{
    char magic[4];
    char filename[64];
    uint32_t payload_size;
};
#pragma pack(pop)

int create_tcp_socket(void);
void handle_nittalk(char **args, int argc);

#endif
