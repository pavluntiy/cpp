#ifndef PROXY_HPP
#define PROXY_HPP

#include <functional>
#include "boost/asio.hpp"
#include <iostream>
#include <exception>
#include <memory>
#include <vector>
#include <map>


class  Connection
{
public:
    using socket_t = boost::asio::ip::tcp::socket;
    using buffer_t = boost::asio::mutable_buffers_1;
    //using buffer_t = boost::asio::streambuf;
    char buff_in[1024];
    char buff_out[1024];
private:

    std::shared_ptr<socket_t> socket;
    //std::shared_ptr<buffer_t> buffer;

public:
    Connection (std::shared_ptr<socket_t> socket):socket(socket)
    {
     //   buffer = std::make_shared<buffer_t>();
        //buffer = std::make_shared<buffer_t>(new char[1024], 10);
        socket->async_read_some(boost::asio::buffer(buff_in, sizeof(buff_in)), std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
    }

    std::shared_ptr<socket_t> get_socket()
    {
        return socket;
    }

    //std::shared_ptr<buffer_t> get_buffer()
    //{
    //    return buffer;
    //}

    void client_read_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
    {
        if (error)
        {
            std::cout << "Error: " << error << "\n";
        }
                               
        std::cout << buff_in;
        //socket->async_read_some(boost::asio::buffer(buff_in, sizeof(buff_in)), std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
       std::cout << bytes_transferred << " LOLLO\n"; 
       //socket->async_read_some(boost::asio::buffer, std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
       //std::cout << buffer.begin() << std::endl;
        //boost::asio::async_read(*socket, *buffer, std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
    }


};


class Proxy
{   
public:
    using acceptor_t = boost::asio::ip::tcp::acceptor; 
    using socket_t = boost::asio::ip::tcp::socket;
    using io_service_t = boost::asio::io_service;
    using endpoint_t = boost::asio::ip::tcp::endpoint;
private:
    
    std::vector<endpoint_t> endpoints;

    //std::vector<std::shared_ptr<socket_t>> slaves;

    std::shared_ptr<acceptor_t> acceptor;
    std::shared_ptr<io_service_t> io_service;
    std::shared_ptr<socket_t> tmp_slave;
    //std::map<socket_t, boost::asio::mutable_buffer> buffers;
    std::vector<Connection> connections;
    
protected:
    void accept_handler(const boost::system::error_code &);
    //void client_read_handler(std::shared_ptr<socket_t>, const boost::system::error_code&, std::size_t);
public:

    Proxy(int, std::vector<endpoint_t>, std::shared_ptr<io_service_t>);
    void run();

};


#endif
