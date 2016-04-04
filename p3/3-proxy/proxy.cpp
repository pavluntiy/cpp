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
    
    //socket_t sock(*io_service);
    endpoint_t endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), port);
    acceptor = std::make_shared<acceptor_t>(*io_service,  endpoint);
    //acceptor->bind(endpoint);
    //acceptor->open(endpoint.protocol());
    acceptor->set_option(acceptor_t::reuse_address(true));
    acceptor->async_accept(*tmp_slave, std::bind(&Proxy::accept_handler, this, std::placeholders::_1)); 
   // acceptor->listen();
   //boost::asio::placeholders::error
}


void Proxy::accept_handler(const boost::system::error_code &error)
{
    // std::cout << acceptor->local_endpoint() << std::endl;
    if(error){
        std::cout << error.message() << std::endl;
    }
    else
    {
       // void *buffer_mem = new char[1024];
       // auto buff = std::make_shared<boost::asio::mutable_buffer>(buffer_mem, sizeof(buffer_mem));
        //slaves.push_back(tmp_slave);
        //boost::asio::async_read(*tmp_slave, *buff, std::bind(&Proxy::client_read_handler, tmp_slave, std::placeholders::_1, std::placeholders::_2)); 
        auto endpoint_connection = std::make_shared<socket_t>(*io_service);
        endpoint_connection->connect(endpoints[0]);
        auto connection_ptr = std::make_shared<Connection>(tmp_slave, endpoint_connection);
        connections.push_back(connection_ptr);
        tmp_slave = std::make_shared<socket_t>(*io_service);
        //acceptor->accept(*tmp_slave);
    }
    // for(auto &it: connections)
    // {

    //     std::cout <<">>>>>> " << &it << " " << it.get_socket() << std::endl;
    // }
    // std::cout << connections.size() << std::endl;

    acceptor->async_accept(*tmp_slave, std::bind(&Proxy::accept_handler, this, std::placeholders::_1)); 
}


void Proxy::run()
{
    io_service->run();  
}

//Connection::Connection (socket_t socket):socket(socket)
Connection::Connection (std::shared_ptr<socket_t> read_socket, std::shared_ptr<socket_t> write_socket):
    read_socket(read_socket),
    write_socket(write_socket)
{
    // if(!read_socket->is_open() || !write_socket->is_open())
    // {

    // }

 //   this->socket = socket;
   // std::cout << this->socket << std::endl;
        // this->output_socket()
        this->read_socket->async_read_some(
                    boost::asio::buffer(buff_in, sizeof(buff_in)), 
                    std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2));

        this->write_socket->async_read_some(
                    boost::asio::buffer(buff_in, sizeof(buff_in)), 
                    std::bind(&Connection::server_read_handler, this,  std::placeholders::_1, std::placeholders::_2));
   // std::cout << "In constr " << this->socket << std::endl;
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
        // if(error == boost::asio::)
        std::cout << "Read error, server: " << error << "\n";
        // read_socket->shutdown(shutdown_types::shutdown_both);
        shutdown();
    }
    else {
        // std::string outbuf(buff_in);
        
        read_socket->async_write_some(boost::asio::buffer(buff_in, sizeof(buff_in)), std::bind(&Connection::server_write_handler, this,  std::placeholders::_1, std::placeholders::_2));
        write_socket->async_read_some(boost::asio::buffer(buff_in, sizeof(buff_in)), std::bind(&Connection::server_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
    }
}


void Connection::client_read_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error)
    {   
        shutdown();
        // boost::system::error_code ec;
        // write_socket->shutdown(shutdown_types::shutdown_both, ec);
        // read_socket->shutdown(shutdown_types::shutdown_both, ec);
        // std::cout << "Read error, client: " << error << "\n";
    }
    else {
        write_socket->async_write_some(boost::asio::buffer(buff_in, sizeof(buff_in)), std::bind(&Connection::client_write_handler, this,  std::placeholders::_1, std::placeholders::_2));
        read_socket->async_read_some(boost::asio::buffer(buff_in, sizeof(buff_in)), std::bind(&Connection::client_read_handler, this,  std::placeholders::_1, std::placeholders::_2)); 
    }
}


void Connection::server_write_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error)
    {
        // read_socket->shutdown(shutdown_types::shutdown_both);
        // write_socket->shutdown(shutdown_types::shutdown_both);
        shutdown();
        std::cout << "Error writing to server: " << error << "\n";
    }
}


void Connection::client_write_handler(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(error)
    {
        // read_socket->shutdown(shutdown_types::shutdown_both);
        // write_socket->shutdown(shutdown_types::shutdown_both);
        shutdown();
        std::cout << "Error writing to client: " << error << "\n";
    }
}


