#ifndef PROXY_HPP
#define PROXY_HPP

#include <functional>
#include "boost/asio.hpp"
#include <iostream>
#include <exception>
#include <memory>
#include <vector>
#include <map>
#include <boost/random/uniform_int_distribution.hpp>


class  Connection
{
public:
    using socket_t = boost::asio::ip::tcp::socket;
    using buffer_t = boost::asio::mutable_buffers_1;
    using endpoint_t = boost::asio::ip::tcp::endpoint;

    using shutdown_types = boost::asio::ip::tcp::socket;
    std::vector<char> buff_server;
    std::vector<char> buff_client;

    std::string server_accum;
    std::string client_accum;

private:

    std::shared_ptr<socket_t> read_socket;
    std::shared_ptr<socket_t> write_socket;
//      socket_t socket;
    const int buff_size = 1024;
    void shutdown();

public:
    Connection (std::shared_ptr<socket_t> read_socket, std::shared_ptr<socket_t> write_socket);
    std::shared_ptr<socket_t> get_read_socket();
    std::shared_ptr<socket_t> get_write_socket();

    void client_read_handler(const boost::system::error_code& error, std::size_t bytes_transferred);
    void server_read_handler(const boost::system::error_code& error, std::size_t bytes_transferred);

    void client_write_handler(const boost::system::error_code& error, std::size_t bytes_transferred);
    void server_write_handler(const boost::system::error_code& error, std::size_t bytes_transferred);

};


class Proxy
{   
        int cur_val = 0;
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
    std::vector<std::shared_ptr<Connection>> connections;

    boost::random::uniform_int_distribution<> dist;
    std::mt19937 gen;
    
protected:
    void accept_handler(const boost::system::error_code &);
    //void client_read_handler(std::shared_ptr<socket_t>, const boost::system::error_code&, std::size_t);
public:

    Proxy(int, std::vector<endpoint_t>, std::shared_ptr<io_service_t>);
    void run();

};


#endif
