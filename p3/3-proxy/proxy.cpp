#include "proxy.hpp"

#include <functional>
#include "boost/asio.hpp"
#include <fstream>
#include <sstream>
#include <memory>

Proxy::Proxy (int port, std::vector<endpoint_t> endpoints, std::shared_ptr<io_service_t> io_service): io_service(io_service)
{
    
    this->endpoints = endpoints;
    tmp_slave = std::make_shared<socket_t>(*io_service);
    
    endpoint_t endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), port);
    acceptor = std::make_shared<acceptor_t>(*io_service,  endpoint);
    acceptor->set_option(acceptor_t::reuse_address(true));
    acceptor->async_accept(*tmp_slave, std::bind(&Proxy::accept_handler, this, std::placeholders::_1)); 
    dist = boost::random::uniform_int_distribution<>(0, endpoints.size());
}


void Proxy::accept_handler(const boost::system::error_code &error)
{
    if(error){
        std::cout << error.message() << std::endl;
    }
    else
    {
        auto endpoint = endpoints[dist(gen)];
        try
        {
            auto endpoint_connection = std::make_shared<socket_t>(*io_service);
            endpoint_connection->connect(endpoint);
            auto connection_ptr = std::make_shared<Connection>(tmp_slave, endpoint_connection);
            connections.push_back(connection_ptr);
        }
        catch(...)
        {
            std::cout << "Failed to connect to " << endpoint << std::endl;
        }
        tmp_slave = std::make_shared<socket_t>(*io_service);

    }

    acceptor->async_accept(*tmp_slave, std::bind(&Proxy::accept_handler, this, std::placeholders::_1)); 
}


void Proxy::run()
{
    io_service->run();  
}

Connection::Connection (std::shared_ptr<socket_t> read_socket, std::shared_ptr<socket_t> write_socket):
    read_socket(read_socket),
    write_socket(write_socket)
{
        buff_in  = std::vector<char>(1024);
        buff_out = std::vector<char>(1024);
        this->read_socket->async_read_some(
                    boost::asio::buffer(buff_in, buff_in.capacity()), 
                    std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2));

        this->write_socket->async_read_some(
                    boost::asio::buffer(buff_out, buff_out.capacity()), 
                    std::bind(&Connection::server_read_handler, this,  std::placeholders::_1, std::placeholders::_2));
}

std::shared_ptr<Connection::socket_t> Connection::get_read_socket()
{
    return read_socket;
}

std::shared_ptr<Connection::socket_t> Connection::get_write_socket()
{
    return write_socket;
}

void Connection::shutdown()
{
    boost::system::error_code ec;
    write_socket->shutdown(shutdown_types::shutdown_both, ec);
    read_socket->shutdown(shutdown_types::shutdown_both, ec);
}

void Connection::server_read_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error)
    {
        std::cout << "Read error, server: " << error << "\n";
        shutdown();
    }
    else 
    {
        auto tmp_memory = std::vector<char>(buff_out);
        read_socket->async_write_some(boost::asio::buffer(tmp_memory), std::bind(&Connection::server_write_handler, this,  std::placeholders::_1, std::placeholders::_2));
        write_socket->async_read_some(boost::asio::buffer(buff_out, buff_out.capacity()), std::bind(&Connection::server_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
    }
}


void Connection::client_read_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error)
    {   
        shutdown();
    }
    else 
    {
        auto tmp_memory = std::vector<char>(buff_in);
        write_socket->async_write_some(boost::asio::buffer(tmp_memory), std::bind(&Connection::client_write_handler, this,  std::placeholders::_1, std::placeholders::_2));
        read_socket->async_read_some(boost::asio::buffer(buff_in, buff_in.capacity()), std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
    }
}


void Connection::server_write_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error)
    {
        shutdown();
        std::cout << "Error writing to server: " << error << "\n";
    }
}


void Connection::client_write_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error)
    {
        shutdown();
        std::cout << "Error writing to client: " << error << "\n";
    }
}


