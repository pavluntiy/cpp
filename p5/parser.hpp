#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cctype>

#include "cmd.hpp"

using namespace std;

class Parser
{
    vector<string> tokens;

    int pos;

    void tokenize(string);

public:

    bool is_op(string);

    unique_ptr<Cmd> parse(string);
    unique_ptr<Cmd> parse_simple_cmd();
};

#endif