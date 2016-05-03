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