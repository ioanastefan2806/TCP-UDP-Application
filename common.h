#ifndef COMMON_H__
#define COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <cstdint>

int send_all(int sockfd, void *buff, size_t len);
int recv_all(int sockfd, void *buff, size_t len);

// structura pentru comunicare TCP/UDP topic-based (mesaje)
struct message {
    char topic[50];
    u_int8_t data_type;
    char content[1500];
} __attribute__((packed));

// structura pentru comunicare de control (stabilirea conexiunii, subscribe, unsubscribe)
struct control_message {
    // 3 eroare -> clientul deja exista
    // 0 stabiliste conexiunea, in data este id-ul clientului
    // 1 subscribe, in data este topicul
    // 2 unsubscribe, in data este topicul
    u_int8_t command_type;
    char data[51];
} __attribute__((packed));


#endif
