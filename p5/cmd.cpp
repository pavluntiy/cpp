#include "cmd.hpp"

void Cmd::run()
{   
    auto vargs = const_cast<const char**>(new char *[args.size() + 1 + 1]);

    vargs[0] = command.c_str();

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