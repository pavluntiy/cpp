#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstdio>
#include <map>
#include <string>

#include "baseserver.hpp"

class Server: public BaseServer
{	



	void broadcast(std::string msg);

	virtual void listener_event(MySocket &socket) override;
	virtual void error_event(MySocket &socket) override;
	virtual void disconnect_event(MySocket &socket) override;
	virtual void read_event(MySocket &socket) override;

	void process_msg(std::string, MySocket&);
    std::string get_next_message(MySocket&);

public:
	Server (int port);
};

#endif