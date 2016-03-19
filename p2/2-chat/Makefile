

all: chatsrv chatclnt


chatsrv: server_main.o server.o baseserver.o my_socket.o
	g++ -std=c++11 -Wall -Wextra -Ditasarom $^ -o $@


server_main.o: server_main.cpp
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom $^ -o $@

server.o: server.cpp 
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom $^ -o $@

baseserver.o: baseserver.cpp
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom $^ -o $@



chatclnt: client_main.o client.o my_socket.o
	g++ -std=c++11  -Wall -Wextra -Ditasarom $^ -o $@

client.o: client.cpp
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom $^ -o $@

client_main.o: client_main.cpp
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom $^ -o $@

my_socket.o: my_socket.cpp
	g++ -std=c++11 -c -Wall -Wextra -Ditasarom $^ -o $@

clean: 
	rm -f *.o