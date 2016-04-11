#include "client.hpp"
#include <iostream>

int main(int argc, char **argv)
{   
    try
    {
        Client client("127.0.0.1", 3000);
        client.run();
    }
    catch (ClientException e)
    {
        std::cout << e.what() << std::endl;
    }
}
