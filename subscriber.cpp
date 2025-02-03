#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <list>
#include <string.h>

#include "common.h"
#include "helpers.h"
#include "utils_sub.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        DIE(true, "utilizare invalida a subscriberului\n");
    }

    // iau id-ul clientului
    int id = atoi(argv[1] + 1);

    // creez socket-ul tcp
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "eroare socket");

    // setez informatiile pentru connect
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    int ret = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(ret == 0, "eroare convertire adresa");

    // conectez clientul la server
    ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "eroare connect");

    // dezactivez Nagle
    int val = 1;
    int opt_ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int));
    DIE(opt_ret < 0, "eroare setare opt socket");


    // trimit mesajul de initializare - id
    struct control_message mesaj_init;
    mesaj_init.command_type = 0;
    memcpy(mesaj_init.data, &id, sizeof(int));

    // dezactivez buffering-ul de pe stdout
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    ret = send_all(sockfd, &mesaj_init, sizeof(struct control_message));
    DIE(ret < 0, "err send init");

    // primesc mesajul de confirmare / eroare
    ret = recv_all(sockfd, &mesaj_init, sizeof(struct control_message));
    DIE(ret < 0, "err recv init");

    if (mesaj_init.command_type == 3) {
        return 0;
    }

    // creez lista de pollfd
    vector<struct pollfd> poll_list;
    poll_list.push_back({sockfd, POLLIN, 0});
    poll_list.push_back({0, POLLIN, 0});

    while (1) {
        // fac poll
        ret = poll(poll_list.data(), poll_list.size(), -1);
        DIE(ret < 0, "err poll");

        // daca am primit de la server
        if (poll_list[0].revents & POLLIN) {
            struct message mesaj;
            memset(&mesaj, 0, sizeof(struct message));
            ret = recv_all(sockfd, &mesaj, sizeof(struct message));
            DIE(ret < 0, "err recv serv");

            if (ret == 0) {
                break;
            }

            char topic[51];
            strncpy(topic, mesaj.topic, 50);
            topic[50] = '\0';

            char* content = mesaj.content;

            string tip = get_type(mesaj.data_type);

            printf("%s - %s - ", topic, tip.c_str());

            if (mesaj.data_type == 0) {
                print_integer(content);
            } else if (mesaj.data_type == 1) {
                print_short_real(content);
            } else if (mesaj.data_type == 2) {
                print_float(content);
            } else if (mesaj.data_type == 3) {
                print_string(content);
            } else {
                cerr << "invalid data type\n";
            }
        }

        // daca am primit de la stdin
        if (poll_list[1].revents & POLLIN) {
            string comanda;
            cin >> comanda;

            if (comanda == "exit") {
                break;
            } else if (comanda == "subscribe") {
                struct control_message mesaj_sub;
                string topic;
                cin >> topic;
                strncpy(mesaj_sub.data, topic.c_str(), 50);
                mesaj_sub.command_type = 1;

                ret = send_all(sockfd, &mesaj_sub, sizeof(mesaj_sub));
                DIE(ret < 0, "err send sub");

                printf("Subscribed to topic %s\n", mesaj_sub.data);
            } else if (comanda == "unsubscribe") {
                string topic;
                struct control_message mesaj_unsub;
                cin >> topic;
                strncpy(mesaj_unsub.data, topic.c_str(), 50);
                mesaj_unsub.command_type = 2;

                ret = send_all(sockfd, &mesaj_unsub, sizeof(mesaj_unsub));
                DIE(ret < 0, "err send unsub");

                printf("Unsubscribed from topic %s\n", mesaj_unsub.data);
            } else {
                cerr << "invalid command\n";
            }
        }
    }

    close(sockfd);

    return 0;
}