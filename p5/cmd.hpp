#ifndef CMD_HPP
#define CMD_HPP

#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include <unistd.h>
#include <sys/fcntl.h>


using namespace std;


class Cmd
{
public:
    string in_file;
    string out_file;

    string command;
    vector<string> args;

    virtual void run();
};


// class LogicCmd: Cmd
// {

// public:
//     unique_ptr<Cmd> left, right;

//     virtual void run() override;
// };

#endif