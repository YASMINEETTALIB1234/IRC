#ifndef JOINCOMMAND_HPP
#define JOINCOMMAND_HPP

#include <vector>
#include <string>

#include "../Server.hpp"

class JoinCommand
{
private:
    static void joinOneChannel(Server &server,
                               Client &client,
                               const std::string &channelName,
                               const std::string &key);

    static void joinManyChannels(Server &server,
                                 Client &client,
                                 const std::vector<std::string> &arguments);
                                 
public:
    static void execute(Server &server,
                        Client &client,
                        const std::vector<std::string> &arguments);

};

#endif