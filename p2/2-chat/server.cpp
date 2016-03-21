
#include "server.hpp"

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
	std::cout << "connection terminated" << std::endl;
	socket.close();
}

void Server::read_event(MySocket &socket)
{
	std::string msg;
	
	socket >> msg;
	// std::cout << "Read ok" << std::endl;

	process_msg(msg, socket); 
	// std::cout << "Concat OK" << std::endl;
	while((msg = get_next_message(socket)) != ""){
		std::cout << "Message from " << socket.sock << ":\n\t" << msg << "\n";
		broadcast(msg);
	}
	
}

void Server::process_msg(std::string msg, MySocket &socket)
{		
	auto &buf = buffers[socket.get_sock()];
	buf += msg; 
	// std::cout << &buffers[socket.get_sock()] << std::endl;
	// auto result = buffers[socket.get_sock()] + msg;
	// buffers[socket.get_sock()] = "";

	// std::cout << "asdfasdfasdffffffffffffffffffffffffffffff" << std::endl;
	// auto position = buf.find('\n');

	// if(position == std::string::npos)
	// {
	// 	return "";
	// }

	// std::string result = buf.substr(0, position + 1);

	// buf = buf.substr(position + 1);

	// return result;
}

std::string Server::get_next_message(MySocket &socket)
{	
	// std::cout << "get_next_message" << std::endl;
	// std::cout << &buffers[socket.get_sock()] << std::endl;
	std::string &buf = buffers.at(socket.get_sock());
	// std::cout << buf.c_str() << std::endl;

	auto position = buf.find('\n');

	// std::cout << position << std::endl;
	// std::cout << buf << std::endl;

	while(position != std::string::npos)
	{		

		std::string result = buf.substr(0, position + 1);
		// std::string result = "OLOOLOLOL";

		if(position + 1  < buf.size()){
			buf = buf.substr(position + 1);
		}
		else
		{
			buffers.at(socket.get_sock()) = "";

			// std::cout << buffers.at(socket.get_sock()) << std::endl;
		}

		return result;
		
	}
	// std::cout << "| "<< &buffers[socket.get_sock()] << std::endl;
	return "";
}



Server::Server (int port = 0): BaseServer(port)
{ 

}

// MySocket& Server::add_socket(int sock)
// {
// 	BaseServer::add_socket(sock);
// 	buffers[sock] = "";
// }







