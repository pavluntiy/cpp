#ifndef CMD_HPP
#define CMD_HPP

#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <signal.h>


using namespace std;


class Cmd
{
public:
    string in_file;
    string out_file;

    string command;
    vector<string> args;

    bool run_background = false;

    virtual void run();
};


class LogicCmd: public Cmd
{

public:
    unique_ptr<Cmd> left, right;
    string type;

    LogicCmd(unique_ptr<Cmd>&&, unique_ptr<Cmd>&&, string);

    virtual void run() override;
};

class PipeCmd: public Cmd
{

public:
    unique_ptr<Cmd> left, right;

    PipeCmd(unique_ptr<Cmd>&&, unique_ptr<Cmd>&&);

    virtual void run() override;
};

#endif