#include "parser.hpp"

void Parser::tokenize(string str)
{
    pos = 0;
    int i = 0;
    while(i < str.size())
    {
        while(i < str.size() && str[i] == ' ')
        {
            ++i;
        }

        string current = "";

        while(i < str.size() && !isspace(str[i]))
        {
            current += str[i];
            ++i;
        }

        tokens.push_back(current);

        // cout << current << endl;
    }
}

bool Parser::is_op(string str)
{
    return str == "&&" || str == "||" || str == "&";
}

unique_ptr<Cmd> Parser::parse(string str)
{

   tokenize(str);

    auto cmd = parse_simple_cmd();
    // while(pos < tokens.size() && is_op(tokens[pos]))
    // {
    //     auto op = tokens[pos];
    //     ++pos;

    //     auto current = parse_simple_cmd();

    // }

    return cmd;
}

unique_ptr<Cmd> Parser::parse_simple_cmd()
{

    vector<string> args;

    auto command = tokens[pos];
    ++pos;

    // cout << command << endl;

    while(pos < tokens.size() && !is_op(tokens[pos]))
    {
        args.push_back(tokens[pos]);
        // cout << tokens[pos] << endl;
        ++pos;
    }

    auto result = make_unique<Cmd>();

    result->args = move(args);
    result->command = move(command);

    

    return move(result);

}