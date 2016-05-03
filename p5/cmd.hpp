#ifndef CMD_HPP
#define CMD_HPP

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

using namespace std;


int default_exit_handler(pid_t pid, int exited_correctly, int exit_status)
{
    cerr << "Process " << pid << " exited: " << exit_status << endl;
    return exit_status;
}


class Cmd
{
public:
    using exit_handler_t = function<int(pid_t, int, int)>;
protected:
    pid_t pid;
    string command;
    vector<string> args;
    const char ** vargs;

    file_descriptor_t stdout;
    file_descriptor_t stdin;

    exit_handler_t exit_handler;
    exit_handler_t ok_handler;
    exit_handler_t fail_handler;

    bool stdin_subst;
    bool stdout_subst;

    virtual int on_exit(bool exited_correctly, int exit_status)
    {
        
        cerr << "Process " << pid << " exited: " << " " << exit_status << endl;
        if(ok_handler && (exited_correctly && !exit_status))
        {   
            // cout << "I'm " << this << endl;
            return ok_handler(pid, exited_correctly, exit_status);
        }

        if(fail_handler && (!exited_correctly || exit_status))
        {
            return fail_handler(pid, exited_correctly, exit_status);
        }

        // if (exit_handler)
        // {        
            
            return exit_status;
        // }

        // return exit_status;
    }

public:

    Cmd()
    {
        stdin_subst = false;
        stdout_subst = false;
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
            ::wait(&wait_status);

            auto exited_correctly = WIFEXITED(wait_status);
            auto exit_status = WEXITSTATUS(wait_status);
       
            return on_exit(exited_correctly, exit_status);
        }
    }

    virtual
    void in_child(){}

    virtual
    void in_parent(){}

    virtual
    int exec()
    {   
            // cout << "Exec " << this << endl;

            if(stdin_subst)
            {   
                ::dup2(stdin, 0);
            }

            if(stdout_subst)
            {
                ::dup2(stdout, 1);
            }

            auto command = args[0];
            cout << command << endl;
            in_child();
            execvp(command.c_str(), const_cast<char *const *>(vargs));
            exit(-1);
            // throw system_error(errno, system_category());
    }

    virtual
    int wait()
    {   
        int wait_status;
        in_parent();
        ::wait(&wait_status);

        auto exited_correctly = WIFEXITED(wait_status);
        auto exit_status = WEXITSTATUS(wait_status);

        
        delete [] vargs;
        return on_exit(exited_correctly, exit_status);
    }

    virtual int run()
    {       

        vargs = const_cast<const char**>(new char *[args.size() + 1]);

        for(int i = 0; i < args.size(); ++i)
        {   
            // cout << args[i] << endl;
            vargs[i] = args[i].c_str();
        }

        vargs[args.size()] = nullptr;
        pid = fork();

        if(!pid)
        {   
            exec();
        }
        else
        {   
            return wait();
        }
    }

 
    // void set_stdin(const Pipe &p)
    // {
    //     stdin_subst = true;
    //     this->stdin = p.get_read();
    // }

    // void set_stdout(const Pipe &p)
    // {
    //     stdout_subst = true;
    //     this->stdin = p.get_write();
    // }

    void interrupt()
    {
        ::kill(pid, SIGINT);
    }

};

class SimpleCmd : public Cmd
{

public:
    SimpleCmd (vector<string> args):Cmd()
    {
        this->args = args;
    }
};

class FileRedirectCmd : public Cmd
{

    Cmd *main_cmd;
    File out;
    File in;

    bool stdout_subst;
    bool stdin_subst;

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
    FileRedirectCmd (Cmd *main_cmd, string redirects_str)
    {   
        this->main_cmd = main_cmd;
        get_redirects(redirects_str);
    }

    virtual
    int run() override
    {
        main_cmd->set_stdin(in);
        main_cmd->set_stdout(out);

        return main_cmd->run();
    }



};


class LogicCmd : public Cmd
{
public:
        enum class Type{AND, OR};
protected:
    Cmd *left;
    Cmd *right;



    Type type;

    vector<FileRedirectCmd*> commands;

    // int or_handler(pid_t pid,  int exited_correctly, int exit_status)
    // {

    //         // cout << "OR HANDLER " << endl;
    //         // cout << &proc << endl;
    //         // Process new_process;
    //         right->set_fail_handler(
    //                 [&]
    //                 (pid_t pid, int exited_correctly, int exit_status) -> int
    //                 {
    //                     return or_handler(pid, exited_correctly, exit_status);
    //                 }
    //         );
            
    //         if(right)
    //         {   
    //             auto tmp = right;
    //             right = nullptr;
    //             tmp->run();
    //             delete tmp;
    //         }

    //         exit(exit_status);
    
    // }

    // int and_handler(Cmd *cmd, pid_t pid,  int exited_correctly, int exit_status)
    // {
    //         // cout << getpid() << " " << pid << endl;
    //     // cout << &proc << endl;
    //         // Process new_process;

    //         // cout << "And handler " << cmd << endl;
    //         // if(!cmd)
    //         // {
    //         //     return exit_status;
    //         // }

    //         // if(dynamic_cast<LogicCmd*>(cmd)){
    //         //     cmd->set_ok_handler(
    //         //             [&]
    //         //             (pid_t pid, int exited_correctly, int exit_status) -> int
    //         //             {   

    //         //                 return and_handler(dynamic_cast<LogicCmd*>(cmd)->right, pid, exited_correctly, exit_status);
    //         //             }
    //         //     );
    //         // }

    //         return right->run();
    //         // return cmd->run();

    //         // }

    //         // exit(exit_status);
    
    // }

public:

    LogicCmd (Cmd *left, Cmd *right, Type type)
    {   
        // cout << "Left " << left << endl;
        // cout << "Right " << right << endl;
        this->left = left;
        this->right = right;
        this->type = type;
    }

    virtual 
    int run() override
    {

        // if(!right)
        // {
        //     return left->run(proc);
        // }
        // if(type == Type::OR){
        //     left->set_fail_handler(
        //                 [&]
        //                 (pid_t pid, int exited_correctly, int exit_status) -> int
        //                 {
        //                     return or_handler(pid, exited_correctly, exit_status);
        //                 }
        //         );
        // }
        // cout << "Running logic "  << this  << endl;

        

        pid = fork();
        if(!pid)
        {   
            if(type == Type::AND){
            // cout << "Setting handler for " << left << endl; 
            left->set_ok_handler(
                        [&]
                        (pid_t pid, int exited_correctly, int exit_status) -> int
                        {   
                            // ::exit(right->run());
                            auto tmp = right->run();
                            cout << "AND " << tmp << endl;
                            ::exit(tmp);
                        }
                );

            left->set_fail_handler(
                        [&]
                        (pid_t pid, int exited_correctly, int exit_status) -> int
                        {   
                            ::exit(exit_status);
                        }
                );
        }

        if(type == Type::OR){
            left->set_ok_handler(
                        [&]
                        (pid_t pid, int exited_correctly, int exit_status) -> int
                        {   
                            ::exit(exit_status);
                        }
                );

            left->set_fail_handler(
                        [&]
                        (pid_t pid, int exited_correctly, int exit_status) -> int
                        {   
                            auto tmp = right->run();
                            cout << "OR " << tmp << endl;
                            ::exit(tmp);
                        }
                );
        }
            left->run();
        }
        else
        {
            // cout << "logic child " << pid << endl;
            return wait();
        }
        // return left->run();

        // return proc.run_function(
        //     [this]
        //     () -> int
        //     {   
        //         Process proc;
        //         // cout << ">>>>>>>>" << &proc << endl;
        //         if(type == Type::OR){
        //             proc.set_fail_handler(
        //                         [&]
        //                         (pid_t pid, int exited_correctly, int exit_status) -> int
        //                         {
        //                             return or_handler(proc, pid, exited_correctly, exit_status);
        //                         }
        //                 );
        //         }

        //         if(type == Type::AND){
        //             proc.set_ok_handler(
        //                         [&]
        //                         (pid_t pid, int exited_correctly, int exit_status) -> int
        //                         {
        //                             return and_handler(proc, pid, exited_correctly, exit_status);
        //                         }
        //                 );
        //         }
        //         return left->run(proc);
        //     }
        // );
    }

};

class LogicExpressionCmd : public Cmd
{

    Cmd *main_cmd;
public:
    LogicExpressionCmd (Cmd* main_cmd)
    {
        this->main_cmd = main_cmd;   
    }

    virtual 
    int run() override
    {
        pid = fork();
        if(!pid)
        {   
            main_cmd->run();
        }
        else
        {
            // cout << "logic child " << pid << endl;
            return wait();
            
        }
    }
    // [this]
        //     () -> int
        //     {   
        //         Process proc;
        //         // cout << ">>>>>>>>" << &proc << endl;
        //         if(type == Type::OR){
        //             proc.set_fail_handler(
        //                         [&]
        //                         (pid_t pid, int exited_correctly, int exit_status) -> int
        //                         {
        //                             return or_handler(proc, pid, exited_correctly, exit_status);
        //                         }
        //                 );
        //         }

        //         if(type == Type::AND){
        //             proc.set_ok_handler(
        //                         [&]
        //                         (pid_t pid, int exited_correctly, int exit_status) -> int
        //                         {
        //                             return and_handler(proc, pid, exited_correctly, exit_status);
        //                         }
        //                 );
        //         }
        //         return left->run(proc);
        //     }
        // );
};

#endif