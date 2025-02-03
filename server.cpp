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
#include <unordered_map>
#include <string.h>
#include <set>

#include "common.h"
#include "helpers.h"
#include "utils_serv.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        DIE(true, "utilizare invalida a serverului\n");
    }

    // map pentru useri - socket, id
    unordered_map<int, int> utilizatori;
    // lista pentru topicuri - topic, lista de useri
    list<pair<string, vector<int>>> topicuri;
    // lista pentru pollfd
    vector<struct pollfd> poll_list;

    // initializez socket-ul tcp
    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_sockfd < 0, "eroare socket tcp");

    // setez informatiile pentru bind
    struct sockaddr_in tcp_addr;
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(atoi(argv[1]));
    tcp_addr.sin_addr.s_addr = INADDR_ANY;

    int bind_ret = bind(tcp_sockfd, (struct sockaddr*) &tcp_addr, sizeof(tcp_addr));
    DIE(bind_ret < 0, "eroare bind tcp");
    int listen_ret = listen(tcp_sockfd, 20);
    DIE(listen_ret < 0, "eroare listen tcp");

    // dezactivez Nagle
    int val = 1;
    int opt_ret = setsockopt(tcp_sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(int));
    DIE(opt_ret < 0, "eroare la setare optiuni tcp");

    // dezactivez buffering-ul de pe stdout
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // initializez socketul udp
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_sockfd < 0, "eroare socket udp");

    int bind_ret_udp = bind(udp_sockfd, (struct sockaddr*) &tcp_addr, sizeof(tcp_addr));
    DIE(bind_ret_udp < 0, "eroare bind udp");

    // adaugam socket-ul tcp in lista de pollfd
    struct pollfd tcp_pollfd;
    tcp_pollfd.fd = tcp_sockfd;
    tcp_pollfd.events = POLLIN;
    poll_list.push_back(tcp_pollfd);

    // adaugam stdin in lista de pollfd
    struct pollfd stdin_pollfd;
    stdin_pollfd.fd = 0;
    stdin_pollfd.events = POLLIN;
    poll_list.push_back(stdin_pollfd);

    // adaugam socket-ul udp in lista de pollfd
    struct pollfd udp_pollfd;
    udp_pollfd.fd = udp_sockfd;
    udp_pollfd.events = POLLIN;
    poll_list.push_back(udp_pollfd);

    while (1) {
        // facem poll pe toti socketii
        int poll_ret = poll(poll_list.data(), poll_list.size(), -1);
        DIE(poll_ret < 0, "err poll");
        
        // verificam daca s-a primit ceva de la stdin
        if (poll_list[1].revents & POLLIN) {
            string comanda;
            cin >> comanda;
            // daca comanda este exit, iesimma
            if (comanda == "exit") {
                break;
            } else {
                cerr << "invalid command\n";
            }
        }

        // verificam daca s-a primit ceva pe socket-ul tcp
        if (poll_list[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_len = sizeof(client_addr);
            int socket_client_nou = accept(tcp_sockfd, (struct sockaddr*) &client_addr, &client_len);
            DIE(socket_client_nou < 0, "err conexiune noua tcp");

            // primim mesajul de initializare
            struct control_message mesaj;
            int recv_ret = recv_all(socket_client_nou, &mesaj, sizeof(struct control_message));
            DIE(recv_ret < 0, "err recv from tcp");

            // verific daca clientul exista deja
            bool exista = user_exists(*(int*)mesaj.data, utilizatori);

            // daca clientul exista deja, ii trimitem un mesaj de eroare
            if (exista) {
                struct control_message mesaj_eroare;
                memset(&mesaj_eroare, 0, sizeof(struct control_message));
                mesaj_eroare.command_type = 3;
                int send_ret = send_all(socket_client_nou, &mesaj_eroare, sizeof(struct control_message));
                DIE(send_ret < 0, "err trimitere la tcp");
                close(socket_client_nou);

                printf("Client C%d already connected.\n", *(int*)mesaj.data);
                continue;
            } else {
                // daca clientul nu exista, ii trimitem un mesaj de confirmare
                struct control_message mesaj_confirmare;
                memset(&mesaj_confirmare, 0, sizeof(struct control_message));
                mesaj_confirmare.command_type = 0;
                int send_ret = send_all(socket_client_nou, &mesaj_confirmare, sizeof(struct control_message));
                DIE(send_ret < 0, "err trimitere la tcp");
            }

            // daca clientul nu exista, il adaugam in lista de clienti
            utilizatori[socket_client_nou] = *(int*)mesaj.data;

            // adaugam noul socket in lista de pollfd
            struct pollfd new_pollfd;
            new_pollfd.fd = socket_client_nou;
            new_pollfd.events = POLLIN;
            poll_list.push_back(new_pollfd);

            printf("New client C%d connected from %s:%d.\n", *(int*)mesaj.data, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // verificam daca s-a primit ceva pe socket-ul udp
        if (poll_list[2].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            struct message mesaj;
            int recv_ret = recvfrom(udp_sockfd, &mesaj, sizeof(mesaj), 0, (struct sockaddr*) &client_addr, &client_len);
            DIE(recv_ret < 0, "err udp msg");

            // cautam toti utilizatorii care sunt abonati la topicul primit
            set<int> clienti;

            for (auto it = topicuri.begin(); it != topicuri.end(); it++) {
                if (topic_inclusion(mesaj.topic, it->first)) {
                    for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                        clienti.insert(*it2);
                    }
                }
            }

            set<int> fds_to_send_to;

            // convertim id-ul in fd (socket)
            for (auto it = clienti.begin(); it != clienti.end(); it++) {
                for (auto it2 = utilizatori.begin(); it2 != utilizatori.end(); it2++) {
                    if (it2->second == *it) {
                        fds_to_send_to.insert(it2->first);
                    }
                }
            }

            // trimitem mesajul la toti clientii abonati
            for (auto it = fds_to_send_to.begin(); it != fds_to_send_to.end(); it++) {
                int send_ret = send_all(*it, &mesaj, sizeof(mesaj));
                DIE(send_ret < 0, "err trimitere tcp");
            }
        }

        // verificam daca s-a primit ceva de la un client
        for (auto it = poll_list.begin() + 3; it != poll_list.end(); it++) {
            if (it->revents & POLLIN) {
                struct control_message mesaj;
                int recv_ret = recv_all(it->fd, &mesaj, sizeof(mesaj));
                DIE(recv_ret < 0, "err primire tcp");

                if (recv_ret == 0) {
                    // clientul s-a deconectat - il scoatem din lista de clienti
                    printf("Client C%d disconnected.\n", utilizatori[it->fd]);
                    utilizatori.erase(it->fd);

                    // il scoatem si din lista de pollfd
                    close(it->fd);
                    poll_list.erase(it);                    
                    break;
                }
                // verifica tipul de mesaj
                if (mesaj.command_type == 1) {
                    // clientul vrea sa se aboneze la un topic
                    string topic = mesaj.data;
                    bool exista = false;
                    for (auto it2 = topicuri.begin(); it2 != topicuri.end(); it2++) {
                        if (it2->first == topic) {
                            exista = true;
                            it2->second.push_back(utilizatori[it->fd]);
                            break;
                        }
                    }

                    if (!exista) {
                        topicuri.push_back({topic, {utilizatori[it->fd]}});
                    }

                } else if (mesaj.command_type == 2) {
                    // clientul vrea sa se dezaboneze de la un topic
                    string topic = mesaj.data;
                    for (auto it2 = topicuri.begin(); it2 != topicuri.end(); it2++) {
                        if (it2->first == topic) {
                            for (auto it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
                                if (*it3 == utilizatori[it->fd]) {
                                    it2->second.erase(it3);
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    
    // inchidem toti socketii si iesim
    for (auto it = poll_list.begin(); it != poll_list.end(); it++) {
        close(it->fd);
    }

    close(tcp_sockfd);
    close(udp_sockfd);

    return 0;
}