#include "shell.hpp"



void sigchld_handler(int signum)
{
    for(auto it = begin(global_shell->background_pids); it != end(global_shell->background_pids); )
    {
        int status;
        if(::waitpid(*it, &status, WNOHANG))
        // if(false)
        {
            auto exited_correctly = WIFEXITED(status);
            auto exit_status = WEXITSTATUS(status);

            cerr << "Process " << *it << " exited: " << exit_status << endl;

            it = global_shell->background_pids.erase(it);
        }
        else
        {
            ++it;
        }

    }
}

void sigint_handler(int signum)
{
    ::signal(SIGINT, sigint_handler);
    global_shell->kill();
}


void Shell::kill()
{
    
    if(current_pid){
        // cout << "Sigint " << current_pid << " in " << ::getpid() << endl;
        ::kill(current_pid, SIGINT);
    }
}

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

        // cout << background_pids.size() << endl;

        // for(auto it = begin(background_pids); it != end(background_pids); )
        // {
        //     int status;
        //     if(::waitpid(*it, &status, WNOHANG))
        //     // if(false)
        //     {
        //         auto exited_correctly = WIFEXITED(status);
        //         auto exit_status = WEXITSTATUS(status);

        //         cerr << "Process " << *it << " exited: " << " " << exit_status << endl;

        //         it = background_pids.erase(it);
        //     }
        //     else
        //     {
        //         ++it;
        //     }

        // }
    }

}

void Shell::spawn(unique_ptr<Cmd> &cmd)
{

    current_pid = ::fork();

    if(!current_pid)
    {
        cmd->run();
    }
    else
    {
        if(!cmd->run_background)
        {   
            // current_pid = pid;
            int status;

            ::wait(&status);
            // if(waitpid(current_pid, &status, 0) <= 0)
            // {
            //     return;
            // }

            auto exited_correctly = WIFEXITED(status);
            auto exit_status = WEXITSTATUS(status);

            cerr << "Process " << current_pid << " exited: "  << exit_status << endl;
        }
        else
        {
            background_pids.push_back(current_pid);
            current_pid = 0;
        }
    }

}