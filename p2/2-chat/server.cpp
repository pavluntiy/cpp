#include "server.hpp"

#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <cstdio>



int main()
{

	int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(3100);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(listener, (sockaddr*) &SockAddr, sizeof(SockAddr));
	listen(listener, SOMAXCONN);


	while(true)
	{
		int slave = accept(listener, 0, 0);
		
		char buf[1024];

		snprintf(buf, sizeof(buf), "%s", "Welcome\0");

		send(slave, buf, sizeof(buf), 0);

	}

}