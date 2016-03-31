#include "boost/asio.hpp"
#include "proxy.hpp"
#include <memory>
#include <vector>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>


int main (int argc, char **argv){
    
    std::vector<Proxy::endpoint_t> endpoints; 

    int port = boost::lexical_cast<int>(argv[1]);


    for(int i = 2; i < argc; ++i)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, argv[i], boost::is_any_of(":"));
        for(auto it: tokens)
        {
            std::cout << it << std::endl;
        }

        endpoints.push_back(Proxy::endpoint_t(boost::asio::ip::address_v4::from_string(tokens[0]), boost::lexical_cast<int>(tokens[1])));
    }
    
    auto io_service = std::make_shared<Proxy::io_service_t>();
    Proxy p(port, endpoints, io_service);
    p.run();
    return 0;
}
