#include "client.hpp"
#include <string>

int main(int argc, char **argv)
{   
    Client client("127.0.0.1", 3100);
    client.run();
}