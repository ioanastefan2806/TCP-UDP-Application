.PHONY: clean server subscriber all

all: server subscriber

server: server.cpp utils_serv.cpp common.c
	g++ -Wall -Wextra -std=c++11 -o server server.cpp utils_serv.cpp common.c

subscriber: subscriber.cpp utils_sub.cpp common.c
	g++ -Wall -Wextra -std=c++11 -o subscriber subscriber.cpp utils_sub.cpp common.c

clean:
	rm -f server subscriber