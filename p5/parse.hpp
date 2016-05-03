#ifndef PARSE_HPP
#define PARSE_HPP

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
#include "cmd.hpp"

using namespace std;



Cmd* parse_simple_cmd(string str)
{
    vector<string> strs;
    vector<string> args;
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

    return new SimpleCmd(args);
}



Cmd* parse_file_redirect_cmd(string str)
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
        return new FileRedirectCmd(parse_simple_cmd(command_str), str.substr(pos, str.size() - pos));
    }
    else
    {
        return parse_simple_cmd(str);
    }

}

Cmd *parse_logic_cmd(string str)
{
    auto pos1 = str.rfind("||");
    auto pos2 = str.rfind("&&");

    auto pos = -1;
    LogicCmd::Type type;
    if(pos1 == -1 && pos2 == -1)
    {
        return parse_file_redirect_cmd(str);
    }

    if(pos1 == -1)
    {
        type = LogicCmd::Type::AND;
        pos = pos2;
    }
    else
    if(pos2 == -1)
    {
        type = LogicCmd::Type::OR;
        pos = pos1;
    }
    else
    {
        pos = max(pos1, pos2);
    }



    auto str_a = str.substr(0, pos);
    auto str_b = str.substr(pos + 2, str.size() - pos - 1);

    // cout << pos << endl;
    // cout << "@@ " << str_a << endl;
    // cout << "** " << str_b << endl;

    auto left = parse_logic_cmd(str_a);
    auto right  = parse_file_redirect_cmd(str_b);
    return new LogicCmd(left, right, type);

}

Cmd* parse_logic_expression_cmd(string str)
{
    auto cmd = parse_logic_cmd(str);
    if(dynamic_cast<LogicCmd*>(cmd))
    {
        return new LogicExpressionCmd(cmd);
    }
    else
    {
        return cmd;
    }
}

Cmd* parse_cmd(string str)
{
    return parse_logic_expression_cmd(str);

}

Cmd *current_proc;

void singal_handler(int signum)
{
    current_proc->interrupt();
}
#endif