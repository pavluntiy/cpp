#include <iostream>

#include "shell.hpp"
#include "cmd.hpp"
#include "parser.hpp"

Shell *global_shell;

int main(void)
{   
    Shell shell;
    global_shell = &shell;

    signal(SIGCHLD, sigchld_handler);

    shell.run();
    return 0;
}