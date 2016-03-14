#include "server.hpp"

#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <cstdio>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>

// std::string read_message(int fd)
// {
// 	while((n = recv(events[i].data.fd, buf, sizeof(buf), MSG_NOSIGNAL)) > 0)
// 	{
// 							msg += std::string(buf);
// 	}
// }

int main()
{

	int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(3100);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(listener, (sockaddr*) &SockAddr, sizeof(SockAddr));

	fcntl(listener, F_SETFL, O_NONBLOCK);
	int optval = 1;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


	listen(listener, SOMAXCONN); 

	const int MAX_EVENTS = 1024;


	int epoll = epoll_create1(0);
	epoll_event *events = (epoll_event*) calloc(MAX_EVENTS, sizeof(epoll_event));

	epoll_event MasterEvent;
	MasterEvent.data.fd = listener;
	MasterEvent.events = EPOLLIN|EPOLLET;

	epoll_ctl(epoll, EPOLL_CTL_ADD, listener, &MasterEvent);


	while(true)
	{

		epoll_event events[MAX_EVENTS];
		auto N = epoll_wait(epoll, events, MAX_EVENTS, -1);

		for(int i = 0; i < N; ++i)
		{
			if((events[i].events & EPOLLERR)||(events[i].events & EPOLLHUP))
    		{
    			// std::cout << "Ololol\n";
    			std::cout << "connection terminated\n";
    			close(events[i].data.fd);
    		}
    		else
			// if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
			{
				if(events[i].data.fd == listener)
				{
					std::cout << "accepted connection" << std::endl;

					int slave = accept(listener, 0, 0);

					epoll_event slave_event;

					slave_event.data.fd = slave;
					slave_event.events = EPOLLIN|EPOLLET;

					epoll_ctl(epoll, EPOLL_CTL_ADD, slave, &slave_event);

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
						// shutdown(events[i].data.fd);
					}
					else
					{	
						std::string msg(buf);
						// std::cout <<"Message from socket " << events[i].data.fd << std::endl;
						// std::cout <<"\tmessage:" << buf << std::endl;

						

						send(events[i].data.fd, buf, msg.size(), MSG_NOSIGNAL);
						std::cout <<"\tmessage:" << msg << std::endl;
						// send(events[i].data.fd, buf, sizeof(buf), MSG_NOSIGNAL);
					}


				}
			}
		}


		//int slave = accept(listener, 0, 0);


		
		// char buf[1024];

		// snprintf(buf, sizeof(buf), "%s", "Welcome\0");

		// send(slave, buf, sizeof(buf), 0);

	}

}