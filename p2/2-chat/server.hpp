#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <cstdio>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>


class Server
{

	int port;
	int listener;
	int epoll;

	epoll_event events[1024];

public:
	Server (int port = 0): port(port)
	{

		this->listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		sockaddr_in SockAddr;
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_port = htons(this->port);
		SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(this->listener, (sockaddr*) &SockAddr, sizeof(SockAddr));

		fcntl(this->listener, F_SETFL, O_NONBLOCK);
		int optval = 1;
		setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


		listen(this->listener, SOMAXCONN); 

		this->epoll = epoll_create1(0);

		epoll_event MasterEvent;
		MasterEvent.data.fd = this->listener;
		MasterEvent.events = EPOLLIN|EPOLLET;

		epoll_ctl(this->epoll, EPOLL_CTL_ADD, this->listener, &MasterEvent);
	}

	void run()
	{

		while(true)
		{

			epoll_event events[1024];
			auto N = epoll_wait(epoll, events, 1024, -1);

			for(int i = 0; i < N; ++i)
			{
				if((events[i].events & EPOLLERR)||(events[i].events & EPOLLHUP))
	    		{
	    			std::cout << "connection terminated\n";
	    			close(events[i].data.fd);
	    		}
	    		else
				{
					if(events[i].data.fd == listener)
					{
						std::cout << "accepted connection" << std::endl;

						int slave = accept(this->listener, 0, 0);

						epoll_event slave_event;

						slave_event.data.fd = slave;
						slave_event.events = EPOLLIN|EPOLLET;

						epoll_ctl(this->epoll, EPOLL_CTL_ADD, slave, &slave_event);

						char msg[] = "Welcome\n";

						send(slave, msg, sizeof(msg), MSG_NOSIGNAL);

						
					}
					else
					{
						char buf[1024];

						int n = recv(events[i].data.fd, buf, sizeof(buf), MSG_NOSIGNAL);
						if(n == 0)
						{
							close(events[i].data.fd);
							std::cout << "connection terminated\n";
						}
						else
						{	
							std::string msg(buf);
							send(events[i].data.fd, buf, msg.size(), MSG_NOSIGNAL);
							std::cout <<"\tmessage:" << msg << std::endl;
						}


					}
				}
			}
		}
	}

};

#endif