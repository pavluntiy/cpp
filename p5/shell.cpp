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

using namespace std;

using file_descriptor_t = int;
using pid_t = int;

class FileException: public exception
{

};

class File
{
    file_descriptor_t fd;
    string fname;

public:
    enum class Mode
    {
        READ,
        WRITE,
        APPEND
    };

    File(string fname, Mode mode = Mode::READ):
        fname(fname)
    {
        if(mode == Mode::READ)
        {
            fd = open(fname.c_str(), O_RDONLY);
        }

        if(mode == Mode::WRITE)
        {
            fd = open(fname.c_str(), O_TRUNC | O_CREAT | O_WRONLY, 0777);
        }

        if(mode == Mode::APPEND)
        {
            fd = open(fname.c_str(), O_WRONLY | O_APPEND);
        }

        if(fd == -1)
        {
            throw system_error(errno, system_category());
        }
    }

    void close()
    {
        ::close(fd);
    }

    file_descriptor_t get_file_descriptor()
    const
    {
        return fd;
    }


};

class Pipe
{
    file_descriptor_t p[2];

    Pipe()
    {
        ::pipe(p);
    }

public:
    file_descriptor_t get_read()
    const
    {
        return p[0];
    }

    file_descriptor_t get_write()
    const
    {
        return p[1];
    }
};

class Process
{
public:
    using exit_handler_t = function<void(pid_t, int, int)>;
protected:
    pid_t pid;
    string command;
    vector<string> args;
    char **vargs;

    file_descriptor_t stdout;
    file_descriptor_t stdin;

    exit_handler_t exit_handler;

    void on_exit(bool exited_correctly, int exit_status)
    {
        if (exit_handler)
        {        
            exit_handler(pid, exited_correctly, exit_status);
        }
    }

public:


    void set_command(string command)
    {
        this->command = command;
    }

    void set_args(vector<string> args)
    {
        this->args = args;
    }

    int run(exit_handler_t exit_handler = exit_handler_t())
    {   
        this->exit_handler = exit_handler;

        int wait_status;
        pid = fork();

        const char ** vargs = const_cast<const char**>(new char *[args.size() + 1]);

        for(int i = 0; i < args.size(); ++i)
        {
            vargs[i] = args[i].c_str();
        }

        vargs[args.size()] = nullptr;

        if(!pid)
        {

            execvp(command.c_str(), const_cast<char *const *>(vargs));
            throw system_error(errno, system_category());
        }
        else
        {
            wait(&wait_status);

            auto exited_correctly = WIFEXITED(wait_status);
            auto exit_status = WEXITSTATUS(wait_status);

            on_exit(exited_correctly, exit_status);
            delete [] vargs;
            return exit_status;
        }
    }

    void set_stdin(const File &f)
    {
        this->stdin = f.get_file_descriptor();
    }

    void set_stdout(const File &f)
    {
        this->stdin = f.get_file_descriptor();
    }

    void set_stdin(const Pipe &p)
    {
        this->stdin = p.get_read();
    }

    void set_stdout(const Pipe &p)
    {
        this->stdin = p.get_write();
    }

    void interrupt()
    {
        ::kill(pid, SIGINT);
    }


};


void default_exit_handler(pid_t pid, int exited_correctly, int exit_status)
{
    cerr << "Process " << pid << " exited: " << exit_status << endl;
}



Process *current_proc;

void singal_handler(int signum)
{
    current_proc->interrupt();
}

int main(void)
{

    Process proc;
    current_proc = &proc;
    
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

        vector<string> strs;
        boost::algorithm::trim(command); 
        boost::split(strs, command, boost::is_any_of("\t \n"));

        
        // cout << "###### " << strs[0] << endl;
        vector<string> args;
        for(auto &it: strs)
        {
            if(it != "")
            {
                args.push_back(it);
            }
        }

        proc.set_command(args[0]);
        proc.set_args(args);
        proc.run(default_exit_handler);
        // cout << ">>>>>> ";
    }
    
}