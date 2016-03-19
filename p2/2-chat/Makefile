

all: chatsrv


chatsrv: server_main.o server.o baseserver.o 
	g++ -std=c++11 -Wall -Wextra -Ditasarom server_main.o server.o baseserver.o -o chatsrv


server_main.o: server.hpp server.cpp baseserver.hpp server.cpp
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom server_main.cpp -o server_main.o

server.o: server.hpp server.cpp 
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom server.cpp -o server.o

baseserver.o: baseserver.hpp baseserver.cpp 
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom baseserver.cpp -o baseserver.o

clean: 
	rm -f *.o