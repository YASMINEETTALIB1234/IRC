#include "../../include/Server.hpp"

// Nombre maximal de channels qu'un client peut rejoindre en meme temps.
// (idealement, ceci devrait etre annonce via le token RPL_ISUPPORT CHANLIMIT)
#define MAX_CHANNELS_PER_CLIENT 10


void Server::joinCommand(Client &client, const std::vector<std::string> &arguments)
{
    // ERR_NEEDMOREPARAMS (461) : il faut au moins un channel en parametre.
    if (arguments.empty())
    {
        std::string reply = ":ircserv 461 " + getClientName(client) +
                             " JOIN :Not enough parameters\r\n";
        sendMessage(client, reply);
        return;
    }
    joinManyChannels(client, arguments);
}


void Server::joinManyChannels(Client &client, const std::vector<std::string> &arguments)
{
    std::vector<std::string> channelNames = split(arguments[0], ',');

    std::vector<std::string> keys;
    if (arguments.size() > 1)
        keys = split(arguments[1], ',');

    for (size_t i = 0; i < channelNames.size(); i++)
    {
        std::string key = "";
        if (i < keys.size())
            key = keys[i];
        joinOneChannel(client, channelNames[i], key);
    }
}

void Server::joinOneChannel(Client &client, const std::string &channelName, const std::string &key)
{
    // ERR_BADCHANMASK (476) : nom de channel syntaxiquement invalide.
    if (!isValidChannelName(channelName))
    {
        std::string reply = ":ircserv 476 " + getClientName(client) + " " +
                             channelName + " :Bad Channel Mask\r\n";
        sendMessage(client, reply);
        return;
    }

    // ERR_TOOMANYCHANNELS (405) : le client a atteint sa limite de channels.
    int joinedCount = 0;
    std::map<std::string, Channel> &channels = getChannels();
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if (it->second.hasMember(&client))
            joinedCount++;
    }

    if (joinedCount >= MAX_CHANNELS_PER_CLIENT)
    {
        std::string reply = ":ircserv 405 " + getClientName(client) + " " +
                             channelName + " :You have joined too many channels\r\n";
        sendMessage(client, reply);
        return;
    }

    // On recupere le channel s'il existe deja, sinon on le cree.
    Channel *channel = findChannel(channelName);
    if (channel == NULL)
        channel = createChannel(channelName);

    // Un client deja membre du channel n'a rien de plus a faire.
    if (channel->hasMember(&client))
        return;
    if (!canJoinChannel(client,*channel,key))
        return;

    joinClient(*channel, client);
    std::string joinMsg = buildCommandReply(client, "JOIN", channel->getName(), "");
    broadcast(*channel,joinMsg,NULL);
    sendTopicReply(client, *channel);
    sendNamesReply(client, *channel);
}
