#ifndef PIPE_HPP
#define PIPE_HPP

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

using namespace std;

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

#endif