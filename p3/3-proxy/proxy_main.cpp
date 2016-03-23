#include "boost/asio.hpp"
#include "proxy.hpp"
#include <memory.h>

int main (int argc, char **argv){

    auto io_service = std::make_shared<Proxy::io_service_t>();
    Proxy p("", io_service);
    p.run();
    return 0;
}
