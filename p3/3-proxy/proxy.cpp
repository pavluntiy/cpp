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
    std::cout << acceptor->local_endpoint() << std::endl;
    if(error){
        std::cout << error.message() << std::endl;
    }
    
    {
       // void *buffer_mem = new char[1024];
       // auto buff = std::make_shared<boost::asio::mutable_buffer>(buffer_mem, sizeof(buffer_mem));
        //slaves.push_back(tmp_slave);
        //boost::asio::async_read(*tmp_slave, *buff, std::bind(&Proxy::client_read_handler, tmp_slave, std::placeholders::_1, std::placeholders::_2)); 
        auto connection = Connection(tmp_slave);
        connections.push_back(connection);
        tmp_slave = std::make_shared<socket_t>(*io_service);
        //acceptor->accept(*tmp_slave);
    }

    acceptor->async_accept(*tmp_slave, std::bind(&Proxy::accept_handler, this, std::placeholders::_1)); 
}


void Proxy::run()
{
    io_service->run();  
}
