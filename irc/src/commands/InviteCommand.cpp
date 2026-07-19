#include "../../include/Server.hpp"

void Server::inviteCommand(Client &client, const std::vector<std::string> &arguments)
{
    // ERR_NEEDMOREPARAMS (461)
    if (arguments.size() < 2)
    {
        sendMessage(client, ":ircserv 461 " + client.getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    std::string targetNick = arguments[0];
    std::string channelName = arguments[1];

    // ERR_NOSUCHCHANNEL (403)
    Channel *channel = findChannel(channelName);
    if (!channel)
    {
        sendMessage(client, ":ircserv 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // ERR_NOTONCHANNEL (442) - issuer must be a member
    if (!channel->hasMember(&client))
    {
        sendMessage(client, ":ircserv 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // target user must exist
    Client *targetClient = findClientByNickname(targetNick);
    if (!targetClient)
    {
        sendMessage(client, ":ircserv 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

    // ERR_USERONCHANNEL (443)
    if (channel->hasMember(targetClient))
    {
        sendMessage(client, ":ircserv 443 " + client.getNickname() + " " + targetNick + " " +
                             channelName + " :is already on channel\r\n");
        return;
    }

    // ERR_CHANOPRIVSNEEDED (482) - only enforced when channel is invite-only
    if (channel->isInviteOnly() && !channel->isOperator(&client))
    {
        sendMessage(client, ":ircserv 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // success: remember the invite so JOIN can check it later on +i channels
    channel->addInvite(targetNick);

    // RPL_INVITING (341) to the issuer
    sendMessage(client, ":ircserv 341 " + client.getNickname() + " " + targetNick + " " + channelName + "\r\n");

    // INVITE message to the target, issuer as source
    std::string inviteMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" +
                             client.getHost() + " INVITE " + targetNick + " " + channelName + "\r\n";
    sendMessage(*targetClient, inviteMsg);
}