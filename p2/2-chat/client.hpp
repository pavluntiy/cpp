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

#include <exception>

#include "my_socket.hpp"


class ClientException: public std::exception
{
    std::string str;
public:
    ClientException(std::string);
    const char* what() const noexcept;
};

class Client
{
    MySocket socket;
    std::string ip;
    int port;

    struct pollfd fds[3];

    void connect();
    void init_poll();

    virtual void socket_read_event();
    virtual void stdin_event();
    virtual void error_event();
    virtual void disconnect_event();
public:
    Client (std::string, int);
    void run();

};



#endif