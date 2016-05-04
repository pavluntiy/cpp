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

void sigchld_handler(int);
void sigint_handler(int);

class Shell
{   


    

    void spawn(unique_ptr<Cmd>&);

public:
    int current_pid = 0;
    void run();
    void kill();
    vector<int> background_pids;
 
};

extern Shell *global_shell;

#endif