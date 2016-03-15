

all: chatsrv


chatsrv: server.o
	g++ -std=c++11 -Wall -Wextra -Ditasarom server.o -o chatsrv


server.o: server.hpp server.cpp baseserver.hpp server.cpp
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom server.cpp -o server.o
