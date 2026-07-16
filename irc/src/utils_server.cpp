#include "../include/Server.hpp"


void Server::sendRPLWelcome(Client &client)
{
    std::string reply =
    ":ircserv 001 " + client.getNickname() + " :Welcome to the ft_irc Network, " + client.getNickname() + "~" + client.getUsername() + "@" + client.getHost() + "\r\n";
    send(
        client.getFd(),
        reply.c_str(),
        reply.length(),
        0
    );
}

void Server::sendRPLYourHost(Client &client)
{
    std::string reply = ":ircserv 002 " + client.getNickname() + " :Your host is ircserv, running version 1.0\r\n";
    send(
        client.getFd(),
        reply.c_str(),
        reply.length(),
        0
    );
}

void Server::sendRPLCreated(Client &client)
{
    std::string reply = ":ircserv 003 " + client.getNickname() + " :This server was created July 2026\r\n";
    send(
        client.getFd(),
        reply.c_str(),
        reply.length(),
        0
    );
}
void Server::sendRPLMyInfo(Client &client)
{
    std::string reply = ":ircserv 004 " + client.getNickname() + " ircserv 1.0 o itkol\r\n";
    send(
        client.getFd(),
        reply.c_str(),
        reply.length(),
        0
    );
}

void Server::sendWelcome(Client &client)
{
    sendRPLWelcome(client);
    sendRPLYourHost(client);
    sendRPLCreated(client);
    sendRPLMyInfo(client);
}

bool Server::isValidNickname(const std::string &nickname)
{
    if (nickname.empty())
        return false;

    if (std::isdigit(nickname[0]) || nickname[0] == '#' || nickname[0] == ':' || nickname[0] == ' ')
        return false;

    for (size_t i = 0; i < nickname.size(); i++)
    {
        char c = nickname[i];
        if (std::isalnum(c))
            continue;
        if (c == '[' || c == ']' || c == '{' || c == '}' || c == '\\' || c == '|')
            continue;
        return false;
    }
    return true;
}

std::string Server::getClientName(Client &client)
{
    if (client.getNickname().empty())
        return "*";
    return client.getNickname();
}

std::map<std::string, Channel> &Server::getChannels()
{
    return channels;
}

bool Server::isRegistered(Client &client)
{
    if (!client.isAuthenticated())
        return false;

    if (client.getNickname().empty())
        return false;

    if (client.getUsername().empty())
        return false;

    return true;
}
