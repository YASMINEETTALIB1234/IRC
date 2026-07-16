#include "../include/Parser.hpp"

Parser::Parser()
{
}

Parser::~Parser()
{
}

void Parser::clear()
{
    command.clear();
    arguments.clear();
}


void Parser::parse(const std::string &message)
{
    clear();

    std::stringstream ss(message);
    std::string word;

    if (!(ss >> command))
        return;

    while (ss >> word)
    {
        if (word[0] == ':')
        {
            word.erase(0, 1);
            std::string rest;
            std::getline(ss, rest);
            word += rest;
            arguments.push_back(word);
            break;
        }

        arguments.push_back(word);
    }
}

std::string Parser::getCommand() const
{
    return command;
}

std::vector<std::string> Parser::getArguments() const
{
    return arguments;
}