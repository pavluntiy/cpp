#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstdio>

#include <string>

#include "baseserver.hpp"

class Server: public BaseServer
{
	void broadcast(std::string msg);
	void listener_event(MySocket &socket);
	void error_event(MySocket &socket);
	void disconnect_event(MySocket &socket);
	void read_event(MySocket &socket);


public:
	Server (int port);
};

#endif