#include "baseserver.hpp"
#include "my_socket.hpp"

void BaseServer::init_listener()
{
	this->listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(this->port);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	fcntl(this->listener, F_SETFL, O_NONBLOCK);
	int optval = 1;
	setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	bind(this->listener, (sockaddr*) &SockAddr, sizeof(SockAddr));

}

void BaseServer::init_epoll()
{
	this->epoll = epoll_create1(0);

	epoll_event MasterEvent;
	MasterEvent.data.fd = this->listener;
	MasterEvent.events = EPOLLIN|EPOLLET;

	epoll_ctl(this->epoll, EPOLL_CTL_ADD, this->listener, &MasterEvent);
}

MySocket& BaseServer::add_socket(int sock)
{	
	fcntl(sock, F_SETFL, O_NONBLOCK);
	epoll_event slave_event;
	slave_event.data.fd = sock;
	slave_event.events = EPOLLIN|EPOLLET;
	epoll_ctl(this->epoll, EPOLL_CTL_ADD, sock, &slave_event);
	this->sockets[sock] = MySocket(sock); 

	buffers[sock] = "";

	return this->sockets[sock];
}

void BaseServer::listener_event(MySocket &socket)
{
	std::cout << "ERROR\n";
}

void BaseServer::error_event(MySocket &socket)
{
	std::cout << "ERROR\n";
}

void BaseServer::disconnect_event(MySocket &socket)
{
	std::cout << "ERROR\n";
}

void BaseServer::read_event(MySocket &socket)
{
	std::cout << "ERROR\n";
}


BaseServer::BaseServer (int port = 0): port(port)
{
	init_listener();
	init_epoll();
	listen(this->listener, SOMAXCONN); 
}


void BaseServer::run()
{

	while(true)
	{

		epoll_event events[1024];
		auto N = epoll_wait(epoll, events, 1024, -1);

		for(int i = 0; i < N; ++i)
		{
			if((events[i].events & EPOLLERR)||(events[i].events & EPOLLHUP))
    		{
    			error_event(this->sockets[events[i].data.fd]);
    		}
    		else
			{
				if(events[i].data.fd == this->listener)
				{
					auto &active_socket = add_socket(accept(this->listener, 0, 0));
					listener_event(active_socket);
				}
				else
				{
					auto &active_socket = this->sockets[events[i].data.fd];
					try
					{
						read_event(active_socket);
					}
					catch(SocketException)
					{
						disconnect_event(active_socket);
					}

				}
			}
		}
	}
}



