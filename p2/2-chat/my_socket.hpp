#ifndef MY_SOCKET_HPP
#define MY_SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <cstdio>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>
#include <map>
#include <exception>

class SocketException: public std::exception
{

};

struct MySocket
{
    int sock;
    std::string  in_buf;
    std::string out_buf;

    MySocket(int sock  = -1);


    void close();

    void flush();

    void reset();

    void read();

    int get_sock();

};

MySocket& operator << (MySocket& s, std::string str);

MySocket& operator >> (MySocket& s, std::string &str);
#endif