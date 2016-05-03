#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <system_error>
#include <functional>

#include <boost/algorithm/string.hpp>

#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "file.hpp"
#include "pipe.hpp"
#include "cmd.hpp"
#include "parse.hpp"

using namespace std;


int main(void)
{

    
    
    string command;
    // if(is)
    // cout << ">>>>>> ";
    signal(SIGINT, singal_handler);

    while(getline(cin, command))
    {   
        
        if(command == "")
        {
            continue;
        }

        Cmd *cmd = parse_cmd(command);
        current_proc = cmd;
        cmd->run();
    }
    
}