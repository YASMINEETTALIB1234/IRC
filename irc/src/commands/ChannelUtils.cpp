#include "../../include/Server.hpp"
#include <sstream>
#include <sys/socket.h>

// ============================================================
// Parsing
// ============================================================

// Coupe une chaine du type "a,b,c" en {"a", "b", "c"}
// Contrairement a la lecture par stringstream classique (qui coupe sur les
// espaces), ici on coupe sur un caractere precis (typiquement ',').
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

// ============================================================
// Validation
// ============================================================

// D'apres la RFC / la doc moderne, un nom de channel :
// - commence par '#', '&', '+' ou '!'
// - ne contient ni espace, ni virgule, ni le caractere BELL (0x07)
// - a une longueur raisonnable (<= 50 caracteres)
bool Server::isValidChannelName(const std::string &name)
{
    if (name.empty() || name.size() > 50)
        return false;

    char prefix = name[0];

    if (prefix != '#' && prefix != '&')
        return false;

    // Un channel ne peut pas être seulement "#" ou "&".
    if (name.size() < 2)
        return false;

    // Les caractères interdits dans le nom.
    for (size_t i = 1; i < name.size(); i++)
    {
        char c = name[i];

        if (c == ' ' || c == ',' || c == ':')
            return false;
    }

    return true;
}

// ============================================================
// Channel
// ============================================================

Channel *Server::findChannel(const std::string &name)
{
    std::map<std::string, Channel>::iterator it = channels.find(name);

    if (it == channels.end())
        return NULL;

    return &(it->second);
}

Channel *Server::createChannel(const std::string &name)
{
    channels[name] = Channel(name);
    return &channels[name];
}

// A appeler AVANT d'ajouter le nouveau membre au channel :
// si le channel n'a encore aucun membre, celui qui rejoint sera le premier
// (et donc, par convention, deviendra operateur du channel).
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

// ============================================================
// Network
// ============================================================

void Server::sendMessage(Client &client, const std::string &message)
{
    send(
        client.getFd(),
        message.c_str(),
        message.length(),
        0
    );
}

void Server::broadcast(Channel &channel, const std::string &message)
{
    std::vector<Client*> &members = channel.getMembers();

    for (size_t i = 0; i < members.size(); i++)
        Server::sendMessage(*members[i], message);
}

// ============================================================
// IRC Replies
// ============================================================

// ":<nick>!<user>@<host> JOIN <channel>\r\n"
std::string Server::buildJoinReply(Client &client, Channel &channel)
{
    return ":" + client.getNickname() + "!" + client.getUsername() +
           "@" + client.getHost() + " JOIN " + channel.getName() + "\r\n";
}

// RPL_NOTOPIC (331) si aucun topic n'est defini, sinon RPL_TOPIC (332).
void Server::sendTopicReply(Client &client, Channel &channel)
{
    if (channel.getTopic().empty())
    {
        std::string reply = ":ircserv 331 " + client.getNickname() + " " +
                             channel.getName() + " :No topic is set\r\n";
        Server::sendMessage(client, reply);
    }
    else
    {
        std::string reply = ":ircserv 332 " + client.getNickname() + " " +
                             channel.getName() + " :" + channel.getTopic() + "\r\n";
        Server::sendMessage(client, reply);
    }
}

// Construit la liste "[prefix]<nick>{ [prefix]<nick>}" utilisee par RPL_NAMREPLY.
// <prefix> est "@" si le membre est operateur du channel, sinon rien.
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

// Envoie RPL_NAMREPLY (353) suivi de RPL_ENDOFNAMES (366).
void Server::sendNamesReply(Client &client, Channel &channel)
{
    std::string namesReply = ":ircserv 353 " + client.getNickname() +
                              " = " + channel.getName() + " :" +
                              Server::buildNamesList(channel) + "\r\n";
    Server::sendMessage(client, namesReply);

    std::string endReply = ":ircserv 366 " + client.getNickname() + " " +
                            channel.getName() + " :End of /NAMES list\r\n";
    Server::sendMessage(client, endReply);
}
