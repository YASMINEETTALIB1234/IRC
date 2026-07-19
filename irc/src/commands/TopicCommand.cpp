#include "../../include/Server.hpp"
#include <ctime>

void Server::topicCommand(Client &client, const std::vector<std::string> &arguments)
{
    if (arguments.empty())
    {
        std::string reply = ":ircserv 461 " + getClientName(client) +
                             " TOPIC :Not enough parameters\r\n";
        sendMessage(client, reply);
        return;
    }

    std::string channelName = arguments[0];
    Channel *channel = findChannel(channelName);

    // ERR_NOSUCHCHANNEL (403)
    if (channel == NULL)
    {
        std::string reply = ":ircserv 403 " + getClientName(client) + " " +
                             channelName + " :No such channel\r\n";
        sendMessage(client, reply);
        return;
    }
    //client non membre de channel (442)
    if (!channel->hasMember(&client))
    {
        std::string reply = ":ircserv 442 " + getClientName(client) + " " +
                             channelName + " :You're not on that channel\r\n";
        sendMessage(client, reply);
        return;
    }

    // --- Cas 1 : consultation (TOPIC name_channel sans arguments :...)
    if (arguments.size() < 2)
    {
        sendTopicReply(client, *channel);
        return;
    }

    // --- Cas 2 : modification (ou effacement si <topic> est vide) ---

    // ERR_CHANOPRIVSNEEDED (482)
    // Si le channel est en mode +t, seul un opérateur peut modifier le topic.
    if (channel->isTopicRestricted() && !channel->isOperator(&client))
    {
        std::string reply = ":ircserv 482 " + getClientName(client) + " " +
                            channelName + " :You're not channel operator\r\n";
        sendMessage(client, reply);
        return;
    }

    std::string newTopic = arguments[1];

    channel->setTopic(newTopic);
    channel->setTopicSetBy(client.getNickname());
    channel->setTopicSetAt(time(NULL));

    // Diffuse a TOUS les membres, auteur du changement inclus.
    std::string topicMsg = buildCommandReply(client, "TOPIC", channel->getName(), channel->getTopic());
    broadcast(*channel, topicMsg, NULL);
}

