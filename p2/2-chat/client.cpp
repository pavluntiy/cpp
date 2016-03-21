
#include <iostream>
#include <poll.h>

#include "client.hpp"
#include "my_socket.hpp"



ClientException::ClientException(std::string str):str(str)
{

}

const char* ClientException::what() const noexcept 
{
    return str.c_str();
}


Client::Client(std::string ip = "", int port = 0):ip(ip), port(port)
{   
    std::cin.sync_with_stdio(false);

    this->connect();
    this->init_poll();


}


void Client::connect()
{
    int socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(this->port);
    SockAddr.sin_addr.s_addr = inet_addr(this->ip.c_str());
    int optval = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    int status = ::connect(socket_fd, (sockaddr*) &SockAddr, sizeof(SockAddr));
    if(status == -1){
        throw ClientException(std::system_error(errno, std::system_category()).what());
    }
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    this->socket = MySocket(socket_fd);

}


void Client::init_poll()
{
    fds[0].fd = this->socket.sock;
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

        for (int i = 0; i < 2; i++) {
            if (fds[i].revents & POLLHUP) {
                std::cout << "ERROR" << std::endl;
                break;
            }

            if (fds[i].revents & POLLIN) {
                if(fds[i].fd == this->socket.sock){
                    try
                    {
                        socket_read_event();
                    }
                    catch(SocketException)
                    {
                        disconnect_event();
                    }
                }
                else
                {   
                    stdin_event();
                }
            }


            
        }

    }

    
}

void Client::socket_read_event()
{

    std::string msg;
    socket >> msg;
    std::cout << msg << std::endl;
}

void Client::stdin_event()
{   
    std::string msg;
    if(std::getline(std::cin, msg)){
        socket << msg + "\n";
        socket.flush();
    }
    else
    {
        socket.close();
        throw ClientException("Eof entered. Exited.");
    }
}

void Client::error_event()
{
    socket.close();
    throw ClientException("Some error occured");
}

void Client::disconnect_event()
{
    socket.close();
    throw ClientException("Lost connection with host");
}