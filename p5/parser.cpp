#include "parser.hpp"

bool Parser::is_letter(char c)
{
    return !(c == '<' || c == '>' || c == '&' || c == '|' || c == ';' || c == ' ' || c == '\n');
}

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

        if(str[i] == '>' || str[i] == '<')
        {
            current = str[i];
            ++i;
        }
        else if(str[i] == '&')
        {
            ++i;
            if(i < str.size() && str[i] == '&')
            {
                current = "&&";
                ++i;
            }
            else
            {
                current = "&";
            }
        }
        else if(str[i] == '|')
        {
            ++i;
            if(i < str.size() && str[i] == '|')
            {
                current = "||";
                ++i;
            }
            else
            {
                current = "|";
            }
        }
        else
        {
            while(i < str.size() && is_letter(str[i]))
            {
                current += str[i];
                ++i;
            }
        }   

        if(current != "")
        {
            tokens.push_back(current);
        }

        // cout << current << endl;
    }
}

bool Parser::is_op(string str)
{
    return str == "&&" || str == "||" || str == "&" || str == "|";
}

bool Parser::is_redirect(string str)
{
    return str == "<" || str == ">";
}

unique_ptr<Cmd> Parser::parse(string str)
{

   tokenize(str);

    unique_ptr<Cmd> cmd = parse_simple_cmd();
    while(pos < tokens.size() && is_op(tokens[pos]))
    {
        auto op = tokens[pos];
        ++pos;

        auto current = parse_simple_cmd();

        if(op == "&&" || op == "||")
        {   
            cmd = make_unique<LogicCmd>(move(cmd), move(current), op);
        }

        if(op == "|")
        {   
            cmd = make_unique<PipeCmd>(move(cmd), move(current));
        }

    }

    return cmd;
}

unique_ptr<Cmd> Parser::parse_simple_cmd()
{

    vector<string> args;

    auto command = tokens[pos];
    ++pos;

    // cout << command << endl;

    while(pos < tokens.size() && !is_op(tokens[pos]) && !is_redirect(tokens[pos]))
    {
        args.push_back(tokens[pos]);
        // cout << tokens[pos] << endl;
        ++pos;
    }

    string in_file = "", out_file = "";
    while(pos < tokens.size() && is_redirect(tokens[pos]))
    {
        auto op = tokens[pos];
        ++pos;
        if(op == ">")
        {
            out_file = tokens[pos]; 
        }
        else
        {
            in_file = tokens[pos]; 
        }

        ++pos;
    }


    auto result = make_unique<Cmd>();

    result->args = move(args);
    result->command = move(command);

    result->in_file = in_file;
    result->out_file = out_file;

    

    return move(result);

}