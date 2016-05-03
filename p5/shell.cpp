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


    }

}

void Shell::spawn(unique_ptr<Cmd> cmd)
{

}