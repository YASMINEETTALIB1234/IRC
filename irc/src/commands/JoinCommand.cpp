#include "../../include/commands/JoinCommand.hpp"
#include "../../include/commands/ChannelUtils.hpp"

// Nombre maximal de channels qu'un client peut rejoindre en meme temps.
// (idealement, ceci devrait etre annonce via le token RPL_ISUPPORT CHANLIMIT)
#define MAX_CHANNELS_PER_CLIENT 10


void JoinCommand::execute(Server &server, Client &client, const std::vector<std::string> &arguments)
{
    // Le client doit etre enregistre (PASS + NICK + USER) avant de pouvoir
    // rejoindre un channel.
    if (!server.isRegistered(client))
    {
        std::string reply = ":ircserv 451 " + server.getClientName(client) +
                             " :You have not registered\r\n";
        ChannelUtils::sendMessage(client, reply);
        return;
    }

    // ERR_NEEDMOREPARAMS (461) : il faut au moins un channel en parametre.
    if (arguments.empty())
    {
        std::string reply = ":ircserv 461 " + server.getClientName(client) +
                             " JOIN :Not enough parameters\r\n";
        ChannelUtils::sendMessage(client, reply);
        return;
    }

    // Argument special "0" : quitter tous les channels rejoints.
    // (le serveur traite cette commande comme si le client avait envoye
    // un PART pour chaque channel dont il est membre)
    if (arguments[0] == "0")
    {
        std::vector<Channel*> &joined = client.getChannels();

        while (!joined.empty())
        {
            Channel *channel = joined[0];

            std::string partMsg = ":" + client.getNickname() + "!" +
                                client.getUsername() + "@" +
                                client.getHost() + " PART " +
                                channel->getName() + " :Leaving\r\n";

            ChannelUtils::broadcast(*channel, partMsg);
            ChannelUtils::leaveClient(server, *channel, client);
        }

        return;
    }

    joinManyChannels(server, client, arguments);
}


void JoinCommand::joinManyChannels(Server &server, Client &client, const std::vector<std::string> &arguments)
{
    std::vector<std::string> channelNames = ChannelUtils::split(arguments[0], ',');

    std::vector<std::string> keys;
    if (arguments.size() > 1)
        keys = ChannelUtils::split(arguments[1], ',');

    for (size_t i = 0; i < channelNames.size(); i++)
    {
        std::string key = "";
        if (i < keys.size())
            key = keys[i];
        joinOneChannel(server, client, channelNames[i], key);
    }
}

void JoinCommand::joinOneChannel(Server &server, Client &client, const std::string &channelName, const std::string &key)
{
    (void)key; // Les modes de channel (+k, +i, +l, +b) ne sont pas encore
               // implementes cote Channel ; ce parametre est conserve pour
               // pouvoir brancher ERR_BADCHANNELKEY / ERR_INVITEONLYCHAN /
               // ERR_BANNEDFROMCHAN / ERR_CHANNELISFULL plus tard.

    // ERR_BADCHANMASK (476) : nom de channel syntaxiquement invalide.
    if (!ChannelUtils::isValidChannelName(channelName))
    {
        std::string reply = ":ircserv 476 " + server.getClientName(client) + " " +
                             channelName + " :Bad Channel Mask\r\n";
        ChannelUtils::sendMessage(client, reply);
        return;
    }

    // ERR_TOOMANYCHANNELS (405) : le client a atteint sa limite de channels.
    int joinedCount = 0;
    std::map<std::string, Channel> &channels = server.getChannels();
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if (it->second.hasMember(&client))
            joinedCount++;
    }

    if (joinedCount >= MAX_CHANNELS_PER_CLIENT)
    {
        std::string reply = ":ircserv 405 " + server.getClientName(client) + " " +
                             channelName + " :You have joined too many channels\r\n";
        ChannelUtils::sendMessage(client, reply);
        return;
    }

    // On recupere le channel s'il existe deja, sinon on le cree.
    Channel *channel = ChannelUtils::findChannel(server, channelName);
    if (channel == NULL)
        channel = ChannelUtils::createChannel(server, channelName);

    // Un client deja membre du channel n'a rien de plus a faire.
    if (channel->hasMember(&client))
        return;

    ChannelUtils::joinClient(*channel, client);

    // 1. Message JOIN, envoye a tous les membres du channel (client inclus,
    //    puisqu'il vient d'y etre ajoute).
    std::string joinMsg = ChannelUtils::buildJoinReply(client, *channel);
    ChannelUtils::broadcast(*channel, joinMsg);

    // 2. Topic du channel (RPL_NOTOPIC ou RPL_TOPIC).
    ChannelUtils::sendTopicReply(client, *channel);

    // 3. Liste des membres du channel (RPL_NAMREPLY + RPL_ENDOFNAMES).
    ChannelUtils::sendNamesReply(client, *channel);
}
