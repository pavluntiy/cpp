#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <cstdio>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <arpa/inet.h>
#include <system_error>
#include <poll.h>


class Client
{
    int socket;
    std::string ip;
    int port;

    struct pollfd fds[3];

    void connect();
    void init_poll();
public:
    Client (std::string, int);
    void run();

};



#endif