#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstdio>

#include <string>

#include "baseserver.hpp"

class Server: public BaseServer
{
	void broadcast(std::string msg);

	virtual void listener_event(MySocket &socket) override;
	virtual void error_event(MySocket &socket) override;
	virtual void disconnect_event(MySocket &socket) override;
	virtual void read_event(MySocket &socket) override;


public:
	Server (int port);
};

#endif