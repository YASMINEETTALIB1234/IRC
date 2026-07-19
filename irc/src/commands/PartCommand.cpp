#include "../../include/Server.hpp"

void Server::partCommand(Client &client,
                         const std::vector<std::string> &arguments)
{
    if (arguments.empty())
    {
        sendMessage(client,
            ":ircserv 461 " +
            client.getNickname() +
            " PART :Not enough parameters\r\n");
        return;
    }
    std::vector<std::string> channels =
        split(arguments[0], ',');

    std::string reason;

    if (arguments.size() == 2)
        reason = arguments[1];

    partManyChannels(
        client,
        channels,
        reason);
}

void Server::partManyChannels(Client &client,
                              const std::vector<std::string> &channels,
                              const std::string &reason)
{
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i].empty())
            continue;

        partOneChannel(
            client,
            channels[i],
            reason);
    }
}

void Server::partOneChannel(Client &client,
                            const std::string &channelName,
                            const std::string &reason)
{
    Channel *channel = findChannel(channelName);

    if (!channel)
    {
        sendMessage(client,
            ":ircserv 403 " +
            client.getNickname() +
            " " +
            channelName +
            " :No such channel\r\n");
        return;
    }
    if (!channel->hasMember(&client))
    {
        sendMessage(client,
            ":ircserv 442 " +
            client.getNickname() +
            " " +
            channelName +
            " :You're not on that channel\r\n");
        return;
    }
    std::string partMsg =
        buildCommandReply(
            client,
            "PART",
            channelName,
            reason);

    broadcast(
        *channel,
        partMsg,
        NULL);

    channel->removeMember(&client);
    if (channel->getMembers().empty())
    {
        channels.erase(channelName);
        return;
    }

    if (channel->getOperators().empty())
    {
        channel->addOperator(channel->getMembers()[0]);
    }
}