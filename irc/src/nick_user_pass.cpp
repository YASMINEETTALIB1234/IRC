#include "../include/Server.hpp"

void Server::handlePass(Client &client, const std::vector<std::string> &arguments)
{
    if (isRegistered(client))
    {
        std::string reply = ":ircserv 462 " + getClientName(client) + " :You may not reregister\r\n";

        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );

        return;
    }

    if (arguments.empty())
    {
        std::string reply = ":ircserv 461 * PASS :Not enough parameters\r\n";
        send(client.getFd(), reply.c_str(), reply.length(), 0);
        return;
    }

    if (arguments[0] == password)
    {
        client.setAuthenticated(true);
        std::cout << "Client authenticated successfully." << std::endl;
    }
    else
    {
        client.setAuthenticated(false); 
        std::string reply =":ircserv 464 * :Password incorrect\r\n";
        send(client.getFd(), reply.c_str(), reply.length(), 0);
        return;
    }
}



//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------



void Server::handleNick(Client &client, const std::vector<std::string> &arguments)
{

    if (!client.isAuthenticated())
    {
        std::string reply = ":ircserv 451 " + getClientName(client) + " :You have not registered\r\n";
        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );
        return;
    }

    if (arguments.empty())
    {
        std::string reply = ":ircserv 431 * :No nickname given\r\n";
        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0);
        return;
    }
    else if (arguments.size() > 1)
    {
        std::string target = getClientName(client);
        std::string reply = ":ircserv 432 " + target + " " + arguments[0] + " :Erroneous nickname\r\n";
        send(client.getFd(),
            reply.c_str(),
            reply.length(),
            0);

        return;
    }

    std::string nickname = arguments[0];
    if (!isValidNickname(nickname))
    {
        std::string target = getClientName(client);
        std::string reply = ":ircserv 432 " + target + " " + nickname + " :Erroneous nickname\r\n";
        send(client.getFd(),
             reply.c_str(),
             reply.length(),
             0);
        return ;
    }

    for (std::map<int, Client>::iterator it = clients.begin();
         it != clients.end();
         ++it)
    {
        if (it->second.getNickname() == arguments[0])
        {
            std::string reply = ":ircserv 433 * " + arguments[0] + " :Nickname is already in use\r\n";
            send(
                client.getFd(),
                reply.c_str(),
                reply.length(),
                0
            );
            return;
        }
    }

    client.setNickname(arguments[0]);
    std::cout << "Nickname set to : "
              << client.getNickname()
              << std::endl;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void Server::handleUser(Client &client, const std::vector<std::string> &arguments)
{
    if (!client.isAuthenticated())
    {
        std::string reply =
            ":ircserv 451 " + getClientName(client) +
            " :You have not registered\r\n";

        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );

        return;
    }

    if (!client.getUsername().empty())
    {
        std::string reply = ":ircserv 462 " + getClientName(client) + " :You may not reregister\r\n";
        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );
        return;
    }
    if (arguments.size() < 4)
    {
        std::string reply =
            ":ircserv 461 USER :Not enough parameters\r\n";

        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );

        return;
    }

    client.setUsername(arguments[0]);

    std::cout << "Username set to : "
              << client.getUsername()
              << std::endl;

    if (isRegistered(client))
    {
        sendWelcome(client);

        std::cout << "Client successfully registered."
                << std::endl;
    }
}