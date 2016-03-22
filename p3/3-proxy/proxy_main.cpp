#include "boost/asio.hpp"
#include "proxy.hpp"

int main (int argc, char **argv){

    boost::asio::io_service io_service;
    Proxy p("", io_service);
    p.run();
    return 0;
}
