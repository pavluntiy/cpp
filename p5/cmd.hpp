#ifndef CMD_HPP
#define CMD_HPP

#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include <unistd.h>


using namespace std;


class Cmd
{
public:
    string file_in;
    string file_out;

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