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

    File()
    {
        fd = -1;
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
    using exit_handler_t = function<int(pid_t, int, int)>;
protected:
    pid_t pid;
    string command;
    vector<string> args;
    char **vargs;

    file_descriptor_t stdout;
    file_descriptor_t stdin;

    exit_handler_t exit_handler;
    exit_handler_t ok_handler;
    exit_handler_t fail_handler;

    bool stdin_subst;
    bool stdout_subst;



    int on_exit(bool exited_correctly, int exit_status)
    {
        
        // cout << "ON EXIT: " << exited_correctly << " " << exit_status << endl;
        
        if(ok_handler && (exited_correctly && !exit_status))
        {
            return ok_handler(pid, exited_correctly, exit_status);
        }

        if(fail_handler && (!exited_correctly || exit_status))
        {
            return fail_handler(pid, exited_correctly, exit_status);
        }

        if (exit_handler)
        {        
            return exit_handler(pid, exited_correctly, exit_status);
        }

        return exit_status;
    }

public:
    Process()
    {
        stdin_subst = false;
        stdout_subst = false;
    }

    void set_ok_handler(exit_handler_t handler)
    {
        ok_handler = handler;
    }

    void set_fail_handler(exit_handler_t handler)
    {
        fail_handler = handler;
    }

    void set_exit_handler(exit_handler_t handler)
    {
        exit_handler = handler;
    }

    void set_command(string command)
    {
        this->command = command;
    }

    void set_args(vector<string> args)
    {
        this->args = args;
    }

    int run_function(function<int()> f)
    {   
        pid = fork();
        exit_handler = exit_handler_t();
        if(!pid)
        {
            // cout << "Running function in " << getpid() << endl;
            exit(f()); 
        }
        else
        {
            // cout << "Waiting  " << pid << " function in " << getpid() << endl;
            int wait_status;
            wait(&wait_status);

            auto exited_correctly = WIFEXITED(wait_status);
            auto exit_status = WEXITSTATUS(wait_status);
       
            return on_exit(exited_correctly, exit_status);
        }
    }

    int run(exit_handler_t exit_handler = exit_handler_t())
    {       
        // cout << "Running process " << this << endl;
        this->exit_handler = exit_handler;

        int wait_status;
        const char ** vargs = const_cast<const char**>(new char *[args.size() + 1]);

        for(int i = 0; i < args.size(); ++i)
        {
            vargs[i] = args[i].c_str();
        }

        vargs[args.size()] = nullptr;
        pid = fork();

        

        if(!pid)
        {   
            
            if(stdin_subst)
            {   
                ::dup2(stdin, 0);
            }

            if(stdout_subst)
            {
                ::dup2(stdout, 1);
            }

            // cout << command << endl;

            execvp(command.c_str(), const_cast<char *const *>(vargs));
            exit(-1);
            throw system_error(errno, system_category());
        }
        else
        {
            wait(&wait_status);

            auto exited_correctly = WIFEXITED(wait_status);
            auto exit_status = WEXITSTATUS(wait_status);

            
            delete [] vargs;
            return on_exit(exited_correctly, exit_status);
        }
    }

    void set_stdin(const File &f)
    {   
        if(f.get_file_descriptor() == -1)
        {
            return;
        }

        stdin_subst = true;
        this->stdin = f.get_file_descriptor();
    }

    void set_stdout(const File &f)
    {
        if(f.get_file_descriptor() == -1)
        {
            return;
        }
        stdout_subst = true;
        this->stdout = f.get_file_descriptor();
    }

    void set_stdin(const Pipe &p)
    {
        stdin_subst = true;
        this->stdin = p.get_read();
    }

    void set_stdout(const Pipe &p)
    {
        stdout_subst = true;
        this->stdin = p.get_write();
    }

    void interrupt()
    {
        ::kill(pid, SIGINT);
    }


};


int default_exit_handler(pid_t pid, int exited_correctly, int exit_status)
{
    cerr << "Process " << pid << " exited: " << exit_status << endl;
    return exit_status;
}



Process *current_proc;

void singal_handler(int signum)
{
    current_proc->interrupt();
}


class Cmd
{
public:
    virtual int run() = 0;
    virtual int run(Process &proc) = 0;
};

class SimpleCmd : public Cmd
{

vector<string> args;
public:
    SimpleCmd (string str)
    {

        vector<string> strs;
        boost::algorithm::trim(str); 
        boost::split(strs, str, boost::is_any_of("\t \n"));  
        for(auto &it: strs)
        {
            boost::trim(it);
            if(it != "")
            {
                args.push_back(it);
            }
        }

    }

    int run(){
        Process proc;
        proc.set_command(args[0]);
        proc.set_args(args);
        return proc.run(default_exit_handler);
    }

    int run(Process &proc){
        proc.set_command(args[0]);
        proc.set_args(args);
        return proc.run(default_exit_handler);
    }

};

class FileRedirectCmd : public Cmd
{

    SimpleCmd *main_cmd;
    File out;
    File in;

    void get_redirects(string str)
    {
        char type = str[0];

        str = str.substr(1);

        int pos = 0;
        bool found = false;
        for(; pos < str.size(); ++pos)
        {
            if(str[pos] == '>' || str[pos] == '<')
            {
                found = true;
                break;
            }
        }

        auto fname = str.substr(0, pos);
        boost::trim(fname);
        if(type == '<')
        {
            in = File(fname, File::Mode::READ);
        }
        else
        {
            out = File(fname, File::Mode::WRITE);
        }

        if(found)
        {
            get_redirects(str.substr(pos, str.size() - pos));
        }

    }

public:
    FileRedirectCmd (string str)
    {

        vector<string> strs;
        boost::algorithm::trim(str);  
        vector<string> trimmed;

        int pos = 0;
        bool found = false;
        for(; pos < str.size(); ++pos)
        {
            if(str[pos] == '>' || str[pos] == '<')
            {
                found = true;
                break;
            }
        }

        string command_str;
        if(found)
        {   
            command_str = str.substr(0, pos);
            get_redirects(str.substr(pos, str.size() - pos));
        }
        else
        {
            command_str = str;
        }

        main_cmd = new SimpleCmd(command_str);

    }

    int run(){
    }

    int run(Process &proc){
        // cout << "Running " << this << endl;
        proc.set_stdin(in);
        proc.set_stdout(out);
        return main_cmd->run(proc);
    }

};


class LogicCmd : public Cmd
{

    FileRedirectCmd *left;
    LogicCmd *right;

    enum class Type{AND, OR};

    Type type;

    vector<FileRedirectCmd*> commands;

    int or_handler(Process &proc, pid_t pid,  int exited_correctly, int exit_status)
    {

            // cout << "OR HANDLER " << endl;
            // cout << &proc << endl;
            Process new_process;
            proc.set_fail_handler(
                    [&]
                    (pid_t pid, int exited_correctly, int exit_status) -> int
                    {
                        return or_handler(new_process, pid, exited_correctly, exit_status);
                    }
            );
            
            if(right)
            {   
                auto tmp = right;
                right = nullptr;
                tmp->run(proc);
                delete tmp;
            }

            exit(exit_status);
    
    }

    int and_handler(Process &proc, pid_t pid,  int exited_correctly, int exit_status)
    {
            // cout << getpid() << " " << pid << endl;
        // cout << &proc << endl;
            Process new_process;
            proc.set_ok_handler(
                    [&]
                    (pid_t pid, int exited_correctly, int exit_status) -> int
                    {
                        return and_handler(new_process, pid, exited_correctly, exit_status);
                    }
            );

            if(right)
            {
                auto tmp = right;
                right = nullptr;
                tmp->run(new_process);
                delete tmp;
            }

            exit(exit_status);
    
    }

public:
    LogicCmd (string str)
    {
        left = nullptr;
        right = nullptr;
        auto pos1 = str.find("||");
        auto pos2 = str.find("&&");

        auto pos = -1;
        if(pos1 == -1 && pos2 == -1)
        {
            left = new FileRedirectCmd(str);
            return;
        }

        if(pos1 == -1)
        {
            type = Type::AND;
            pos = pos2;
        }
        else
        if(pos2 == -1)
        {
            type = Type::OR;
            pos = pos1;
        }
        else
        {
            pos = min(pos1, pos2);
        }



        auto str_a = str.substr(0, pos);
        auto str_b = str.substr(pos + 2, str.size() - pos - 1);

        // cout << pos << endl;
        // cout << "@@ " << str_a << endl;
        // cout << "** " << str_b << endl;

        left = new FileRedirectCmd(str_a);
        right = new LogicCmd(str_b);

    }

    int run(){
    }

    int run(Process &proc){

        if(!right)
        {
            return left->run(proc);
        }


        return proc.run_function(
            [this]
            () -> int
            {   
                Process proc;
                // cout << ">>>>>>>>" << &proc << endl;
                if(type == Type::OR){
                    proc.set_fail_handler(
                                [&]
                                (pid_t pid, int exited_correctly, int exit_status) -> int
                                {
                                    return or_handler(proc, pid, exited_correctly, exit_status);
                                }
                        );
                }

                if(type == Type::AND){
                    proc.set_ok_handler(
                                [&]
                                (pid_t pid, int exited_correctly, int exit_status) -> int
                                {
                                    return and_handler(proc, pid, exited_correctly, exit_status);
                                }
                        );
                }
                return left->run(proc);
            }
        );
    }

};

int main(void)
{

    
    
    string command;
    // if(is)
    // cout << ">>>>>> ";
    signal(SIGINT, singal_handler);

    while(getline(cin, command))
    {   
        Process proc;
        current_proc = &proc;
        if(command == "")
        {
            continue;
        }

        LogicCmd cmd(command);
        cmd.run(proc);
    }
    
}