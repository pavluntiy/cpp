#include "cmd.hpp"

void Cmd::run()
{   
    auto vargs = const_cast<const char**>(new char *[args.size() + 1 + 1]);

    vargs[0] = command.c_str();

    if(out_file != "")
    {
        auto fd = ::open(out_file.c_str(), O_TRUNC | O_CREAT | O_WRONLY, 0777);
        ::dup2(fd, 1);
    }

    if(in_file != "")
    {
        auto fd = open(in_file.c_str(), O_RDONLY);
        ::dup2(fd, 0);
    }


    for(int i = 0; i < args.size(); ++i)
    {   
        vargs[i + 1] = args[i].c_str();
        // cout << args[i];
    }

    vargs[args.size() + 1] = nullptr;

    // cout << command << endl;

    execvp(command.c_str(), const_cast<char *const *>(vargs));
    exit(-1);
}

LogicCmd::LogicCmd(unique_ptr<Cmd> &&left, unique_ptr<Cmd> &&right, string type):left(move(left)), right(move(right)), type(type)
{}

void LogicCmd::run()
{
    int current_pid;
    if(type == "&&")
    {
        if(!(current_pid = fork()))
        {
            left->run();
        }

        int status;

        ::wait(&status);

        auto exited_correctly = WIFEXITED(status);
        auto exit_status = WEXITSTATUS(status);

        cerr << "Process " << current_pid << " exited: " << " " << exit_status << endl;

        if(exited_correctly && !exit_status)
        {
           right->run(); 
        }

        ::exit(exit_status);
    }
    else
    {
        if(!(current_pid = fork()))
        {
            left->run();
        }

        int status;
        
        ::wait(&status);

        auto exited_correctly = WIFEXITED(status);
        auto exit_status = WEXITSTATUS(status);

        cerr << "Process " << current_pid << " exited: " << " " << exit_status << endl;

        if(!exited_correctly || exit_status)
        {
           right->run(); 
        }

        ::exit(exit_status);
    }
}

PipeCmd::PipeCmd(unique_ptr<Cmd> &&left, unique_ptr<Cmd> &&right):left(move(left)), right(move(right))
{}

void PipeCmd::run()
{
    int p[2];
    ::pipe(p);

    int pid1 = ::fork();
    if(!pid1)
    {
        ::close(p[0]);
        ::dup2(p[1], 1);
        ::close(p[1]);
        
        left->run();
    }
    
    int pid2 = ::fork();
    if(!pid2)
    {
        ::close(p[1]);
        ::dup2(p[0], 0);
        ::close(p[0]);
        
        right->run();
    }

    ::close(p[1]);
    ::close(p[0]);

    int status1, status2;
    ::waitpid(pid1, &status1, 0);
    ::waitpid(pid2, &status2, 0);

    auto exited_correctly = WIFEXITED(status2);
    auto exit_status = WEXITSTATUS(status2);

    // cout << status1 << " " << status2 << endl;

    ::exit(exit_status);

}