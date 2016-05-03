#include "shell.hpp"

void Shell::run()
{   
    string str;

    while(getline(cin, str))
    {
        if(str == "")
        {
            continue;
        }

        Parser parser;
        auto cmd = parser.parse(str);

        spawn(cmd);
    }

}

void Shell::spawn(unique_ptr<Cmd> &cmd)
{

    current_pid = fork();

    if(!current_pid)
    {
        cmd->run();
    }
    else
    {
        int status;

        wait(&status);

        auto exited_correctly = WIFEXITED(status);
        auto exit_status = WEXITSTATUS(status);

        cerr << "Process " << current_pid << " exited: " << " " << exit_status << endl;
    }

}