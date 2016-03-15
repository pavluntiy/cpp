#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstdio>

#include <string>

#include "baseserver.hpp"

class Server: public BaseServer
{
	void broadcast(std::string msg)
	{
		for(auto p: this->sockets)
		{	
			p.second << msg;
			p.second.flush();
		}
	}

	void listener_event(MySocket &socket)
	{
		std::cout << "accepted connection" << std::endl;
		socket << "Welcome\n";
		socket.flush();
	}

	void error_event(MySocket &socket)
	{
		std::cout << "ERROR\n";
	   	socket.close();
	}

	void disconnect_event(MySocket &socket)
	{
		std::cout << "connection closed\n";
		socket.close();
	}

	void read_event(MySocket &socket)
	{
		std::string msg;
		socket >> msg;
		std::cout << "Message from " << socket.sock << ":\n\t" << msg << "\n";
		broadcast(msg);
	}



public:
	Server (int port = 0): BaseServer(port)
	{ 
	}


};

#endif