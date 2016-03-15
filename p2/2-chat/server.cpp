
#include "server.h"

void Server::broadcast(std::string msg)
{
	for(auto p: this->sockets)
	{	
		p.second << msg;
		p.second.flush();
	}
}

void Server::listener_event(MySocket &socket)
{
	std::cout << "accepted connection" << std::endl;
	socket << "Welcome\n";
	socket.flush();
}

void Server::error_event(MySocket &socket)
{
	std::cout << "ERROR\n";
   	socket.close();
}

void Server::disconnect_event(MySocket &socket)
{
	std::cout << "connection closed\n";
	socket.close();
}

void Server::read_event(MySocket &socket)
{
	std::string msg;
	socket >> msg;
	std::cout << "Message from " << socket.sock << ":\n\t" << msg << "\n";
	broadcast(msg);
}




Server::Server (int port = 0): BaseServer(port)
{ 
}







