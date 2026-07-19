#include "../../include/Server.hpp"
#include <sstream>
#include <sys/socket.h>

std::vector<std::string> Server::split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    // std::string var;
    // ss >> var;
    // std::cout << var

    while (std::getline(ss, token, delimiter))
    {
        if (!token.empty())
            tokens.push_back(token);
    }

    return tokens;
}

bool Server::isValidChannelName(const std::string &name)
{
    if (name.size() < 2 || name.size() > 50)
        return false;

    if (name[0] != '#' && name[0] != '&')
        return false;

    // Les caractères interdits dans le nom.
    for (size_t i = 1; i < name.size(); i++)
    {
        if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
            return false;
    }

    return true;
}

Channel *Server::findChannel(const std::string &name)
{
    std::map<std::string, Channel>::iterator it = channels.find(name);

    if (it == channels.end())
        return NULL;

    return &(it->second);
}

Channel *Server::createChannel(const std::string &name)
{
    channels[name] = Channel(name);//channels[name] represente value de key name
    return &channels[name];
}

bool Server::isFirstMember(Channel &channel)
{
    return channel.getMembers().empty();
}

void Server::joinClient(Channel &channel, Client &client)
{
    bool wasEmpty = isFirstMember(channel);

    channel.addMember(&client);

    // Le premier client a rejoindre un channel en devient operateur.
    if (wasEmpty)
        channel.addOperator(&client);
}

void Server::sendMessage(Client &client, const std::string &message)
{
    send(
        client.getFd(),
        message.c_str(),
        message.length(),
        0
    );
}

std::string Server::buildNamesList(Channel &channel)
{
    std::vector<Client*> &members = channel.getMembers();
    std::string list;

    for (size_t i = 0; i < members.size(); i++)
    {
        if (!list.empty())
            list += " ";

        if (channel.isOperator(members[i]))
            list += "@";

        list += members[i]->getNickname();
    }

    return list;
}

void Server::sendNamesReply(Client &client, Channel &channel)
{
    std::string namesReply = ":ircserv 353 " + client.getNickname() +
                              " = " + channel.getName() + " :" +
                              buildNamesList(channel) + "\r\n";
    Server::sendMessage(client, namesReply);

    std::string endReply = ":ircserv 366 " + client.getNickname() + " " +
                            channel.getName() + " :End of /NAMES list\r\n";
    Server::sendMessage(client, endReply);
}

void Server::sendTopicReply(Client &client, Channel &channel)
{
    if (channel.getTopic().empty())
    {
        std::string reply = ":ircserv 331 " + client.getNickname() + " " +
                             channel.getName() + " :No topic is set\r\n";
        sendMessage(client, reply);
    }
    else
    {
        std::string reply = ":ircserv 332 " + client.getNickname() + " " +
                             channel.getName() + " :" + channel.getTopic() + "\r\n";
        sendMessage(client, reply);

        // RPL_TOPICWHOTIME (333), seulement si le topic a ete pose via TOPIC
        // (donc que les metadonnees who/when existent).
        if (!channel.getTopicSetBy().empty())
        {
            std::ostringstream oss;
            oss << channel.getTopicSetAt();

            std::string whoTime = ":ircserv 333 " + client.getNickname() + " " +
                                   channel.getName() + " " + channel.getTopicSetBy() +
                                   " " + oss.str() + "\r\n";
            sendMessage(client, whoTime);
        }
    }
}

std::string Server::buildCommandReply(Client &client,
                                      const std::string &command,
                                      const std::string &target,
                                      const std::string &text)
{
    std::string reply = ":" + client.getNickname() + "!" +
                        client.getUsername() + "@" +
                        client.getHost() + " " +
                        command + " " + target;

    if (!text.empty())
        reply += " :" + text;

    reply += "\r\n";

    return reply;
}

Client *Server::findClient(const std::string &nickname)
{
    std::map<int, Client>::iterator it = clients.begin();

    while (it != clients.end())
    {
        if (it->second.getNickname() == nickname)
            return &(it->second);

        ++it;
    }

    return NULL;
}

void Server::broadcast(Channel &channel,
                       const std::string &message,
                       Client *exclude)
{
    std::vector<Client*> &members = channel.getMembers();

    for (size_t i = 0; i < members.size(); i++)
    {
        if (exclude != NULL && members[i] == exclude)// On saute cette personne a exclure
            continue;

        sendMessage(*members[i], message);
    }
}

Client *Server::findClientByNickname(const std::string &nick)
{
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == nick)
            return &(it->second);
    }
    return NULL;
}

bool Server::canJoinChannel(Client &client,
                            Channel &channel,
                            const std::string &key)
{
    // ERR_INVITEONLYCHAN (473)
    if (channel.isInviteOnly())
    {
        if (!channel.isInvited(client.getNickname()))
        {
            sendMessage(client,
                ":ircserv 473 " +
                getClientName(client) +
                " " +
                channel.getName() +
                " :Cannot join channel (+i)\r\n");
            return false;
        }

        // Consommer l'invitation
        channel.removeInvite(client.getNickname());
    }

    // ERR_BADCHANNELKEY (475)
    if (channel.hasKey())
    {
        if (key != channel.getKey())
        {
            sendMessage(client,
                ":ircserv 475 " +
                getClientName(client) +
                " " +
                channel.getName() +
                " :Cannot join channel (+k)\r\n");
            return false;
        }
    }

    // ERR_CHANNELISFULL (471)
    if (channel.hasUserLimit())
    {
        if (channel.getMembers().size() >= channel.getUserLimit())
        {
            sendMessage(client,
                ":ircserv 471 " +
                getClientName(client) +
                " " +
                channel.getName() +
                " :Cannot join channel (+l)\r\n");
            return false;
        }
    }

    return true;
}