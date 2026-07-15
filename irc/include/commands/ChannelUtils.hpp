#ifndef CHANNELUTILS_HPP
#define CHANNELUTILS_HPP

#include "../Server.hpp"
#include "../Channel.hpp"
#include "../Client.hpp"

#include <vector>
#include <string>

class ChannelUtils
{
public:
    // Parsing
    static std::vector<std::string> split(const std::string&, char);

    // Validation
    static bool isValidChannelName(const std::string&);

    // Channel
    static Channel *findChannel(Server&, const std::string&);
    static Channel *createChannel(Server&, const std::string&);
    static bool isFirstMember(Channel&);
    static void joinClient(Channel&, Client&);

    // Network
    static void sendMessage(Client&, const std::string&);
    static void broadcast(Channel&, const std::string&);

    // IRC Replies
    static std::string buildJoinReply(Client&, Channel&);
    static void sendTopicReply(Client&, Channel&);
    static std::string buildNamesList(Channel&);
    static void sendNamesReply(Client&, Channel&);
};

#endif