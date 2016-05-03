#ifndef SHELL_HPP
#define SHELL_HPP

#include <iostream>
#include <vector>
#include <memory>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <signal.h>


#include "cmd.hpp"
#include "parser.hpp"

using namespace std;

class Shell
{   
    int current_pid;

    void spawn(unique_ptr<Cmd>&);

public:
    void run();
 
};

#endif