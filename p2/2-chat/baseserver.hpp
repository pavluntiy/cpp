#ifndef BASE_SERVER_HPP
#define BASE_SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <cstdio>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>
#include <map>
#include <exception>

class SocketException: public std::exception
{

};

struct MySocket
{
	int sock;
	std::string  in_buf;
	std::string out_buf;

	MySocket(int sock);


	void close();

	void flush();

	void reset();

	void read();

};

MySocket& operator << (MySocket& s, std::string str);

MySocket& operator >> (MySocket& s, std::string &str);

class BaseServer
{

	int port;
	int listener;
	int epoll;

	epoll_event events[1024];


	void init_listener();

	void init_epoll();

	MySocket& add_socket(int sock);

	virtual void listener_event(MySocket &socket);

	virtual void error_event(MySocket &socket);

	virtual void disconnect_event(MySocket &socket);

	virtual void read_event(MySocket &socket);

protected:
		std::map<int, MySocket> sockets;

public:
	BaseServer (int port);
	void run();


};

#endif