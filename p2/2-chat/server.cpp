#include "server.hpp"

#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <cstdio>

#include <fcntl.h>
#include <sys/epoll.h>

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
			// if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
			{
				if(events[i].data.fd == listener)
				{
					std::cout << "ACCEPTED CONNECTION" << std::endl;

					int slave = accept(listener, 0, 0);

					epoll_event slave_event;

					slave_event.data.fd = slave;
					slave_event.events = EPOLLIN|EPOLLET;

					epoll_ctl(epoll, EPOLL_CTL_ADD, slave, &slave_event);
				}
				else
				{
					std::cout <<"Message from socket " << events[i].data.fd << std::endl;

					char buf[1024];

					recv(events[i].data.fd, buf, sizeof(buf), MSG_NOSIGNAL);

					std::cout <<"\tmessage:" << buf << std::endl;

					send(events[i].data.fd, buf, sizeof(buf), MSG_NOSIGNAL);


				}
			}
		}


		//int slave = accept(listener, 0, 0);


		
		// char buf[1024];

		// snprintf(buf, sizeof(buf), "%s", "Welcome\0");

		// send(slave, buf, sizeof(buf), 0);

	}

}