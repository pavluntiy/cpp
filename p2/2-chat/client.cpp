
#include <iostream>

#include "client.hpp"
#include <poll.h>

Client::Client(std::string ip = "", int port = 0):ip(ip), port(port)
{   
    std::cin.sync_with_stdio(false);

    this->connect();
    this->init_poll();


}


void Client::connect()
{
    this->socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(this->port);
    SockAddr.sin_addr.s_addr = inet_addr(this->ip.c_str());
    int optval = 1;
    setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    int status = ::connect(this->socket, (sockaddr*) &SockAddr, sizeof(SockAddr));
    if(status == -1){
        throw std::system_error(errno, std::system_category());
    }
    fcntl(this->socket, F_SETFL, O_NONBLOCK);
}


void Client::init_poll()
{
    fds[0].fd = this->socket;
    fds[0].events = POLLIN;

    fds[1].fd = 0;
    fds[1].events = POLLIN;

    fcntl(0, F_SETFL, O_NONBLOCK);

    // fds[1] = stdin
}

void Client::run()
{
    
    while(true)
    {
        int N = poll(this->fds, sizeof(fds)/sizeof(struct pollfd), -1);

        for (int i = 0; i < 1; i++) {
            if (fds[i].revents & POLLHUP) {
                std::cout << "ERROR" << std::endl;
                break;
            }
            if (fds[i].revents & POLLWRBAND) {
    
            }
            if (fds[i].revents & POLLIN) {
                char buf[1024];
                int n = recv(fds[i].fd, buf, sizeof(buf), MSG_NOSIGNAL);
                if(n == 0)
                {
                    break;
                    shutdown(fds[i].fd, SHUT_RDWR);
                    close(fds[i].fd);
                }
                std::cout << std::string(buf) << std::endl;
            }
            
        }

    }

    
}