#include "../../include/Server.hpp"
#include <ctime>

//     Command: TOPIC
//  Parameters: <channel> [<topic>]
void Server::topicCommand(Client &client, const std::vector<std::string> &arguments)
{
    // ERR_NEEDMOREPARAMS (461)
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

    // ERR_NOTONCHANNEL (442) : la reference l'autorise (MAY) pour la
    // consultation ; applique aussi a la modification (comportement
    // standard des ircd : impossible de changer le topic d'un channel
    // auquel on n'a pas rejoint).
    if (!channel->hasMember(&client))
    {
        std::string reply = ":ircserv 442 " + getClientName(client) + " " +
                             channelName + " :You're not on that channel\r\n";
        sendMessage(client, reply);
        return;
    }

    // --- Cas 1 : consultation (pas de parametre <topic>) ---
    if (arguments.size() < 2)
    {
        sendTopicReply(client, *channel);
        return;
    }

    // --- Cas 2 : modification (ou effacement si <topic> est vide) ---
    //
    // TODO: si le mode "protected topic" (+t) est defini sur le channel,
    // verifier ici channel->isOperator(&client), sinon ERR_CHANOPRIVSNEEDED
    // (482). Pas encore applicable : MODE n'existe pas, donc aucun channel
    // ne peut etre +t pour l'instant.

    std::string newTopic = arguments[1];

    channel->setTopic(newTopic);
    channel->setTopicSetBy(client.getNickname());
    channel->setTopicSetAt(time(NULL));

    // Diffuse a TOUS les membres, auteur du changement inclus.
    std::string topicMsg = buildCommandReply(client, "TOPIC", channel->getName(), channel->getTopic());
    broadcast(*channel, topicMsg);
}

