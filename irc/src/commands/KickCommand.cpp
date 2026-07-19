#include "../../include/Server.hpp"

void Server::kickCommand(Client &client, const std::vector<std::string> &arguments)
{
    // ERR_NEEDMOREPARAMS (461)
    if (arguments.size() < 2)
    {
        sendMessage(client, ":ircserv 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
        return;
    }

    std::string channelName = arguments[0];
    std::string userList = arguments[1];

    // comment is everything after the third token (the trailing part, if any)
    std::string comment;
    if (arguments.size() > 2)
        comment = arguments[2];

    // ERR_NOSUCHCHANNEL (403)
    Channel *channel = findChannel(channelName);
    if (!channel)
    {
        sendMessage(client, ":ircserv 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // ERR_NOTONCHANNEL (442) - the kicker must be a member
    if (!channel->hasMember(&client))
    {
        sendMessage(client, ":ircserv 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // ERR_CHANOPRIVSNEEDED (482)
    if (!channel->isOperator(&client))
    {
        sendMessage(client, ":ircserv 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // support comma-separated target list: KICK #ch a,b,c :reason
    std::vector<std::string> targets = split(userList, ',');

    for (size_t i = 0; i < targets.size(); i++)
    {
        std::string targetNick = targets[i];

        Client *targetClient = findClientInChannel(*channel, targetNick);

        // ERR_USERNOTINCHANNEL (441)
        if (!targetClient)
        {
            sendMessage(client, ":ircserv 441 " + client.getNickname() + " " + targetNick + " " +
                                 channelName + " :They aren't on that channel\r\n");
            continue;
        }

        // default comment if none given
        std::string reason = comment.empty() ? targetNick : comment;

        // one KICK message per target, broadcast to everyone in the channel
        std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" +
                               client.getHost() + " KICK " + channelName + " " +
                               targetNick + " :" + reason + "\r\n";
        broadcast(*channel, kickMsg,NULL);

        // actually remove them from the channel (and strip op status if they had it)
        channel->removeMember(targetClient);
    }
}