#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <sstream>


class Parser
{
private:
    std::string command;
    std::vector<std::string> arguments;

public:
    Parser();
    ~Parser();

    void parse(const std::string &message);

    std::string getCommand() const;
    std::vector<std::string> getArguments() const;

    void clear();
};

#endif