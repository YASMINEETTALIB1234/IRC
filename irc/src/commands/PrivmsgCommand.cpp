#include "../../include/Server.hpp"

#define MAX_PRIVMSG_TARGETS 10

void Server::privmsgCommand(Client &client,
                            const std::vector<std::string> &arguments)
{
    if (arguments.empty())
    {
        sendMessage(client,
            ":ircserv 411 " +
            client.getNickname() +
            " :No recipient given (PRIVMSG)\r\n");
        return;
    }

    if (arguments.size() < 2 || arguments[1].empty())//case PRIVMSG RANIA OU PRIVMSG RANIA :
    {
        sendMessage(client,
            ":ircserv 412 " +
            client.getNickname() +
            " :No text to send\r\n");
        return;
    }

    std::vector<std::string> targets =
        split(arguments[0], ',');

    if (targets.size() > MAX_PRIVMSG_TARGETS)
    {
        sendMessage(client,
            ":ircserv 407 " +
            client.getNickname() +
            " :Too many targets\r\n");
        return;
    }

    privmsgManyTargets(
        client,
        targets,
        arguments[1]);
}

void Server::privmsgManyTargets(Client &client,
                                const std::vector<std::string> &targets,
                                const std::string &message)
{
    for (size_t i = 0; i < targets.size(); i++)
    {
        // Ignorer un target vide (ex: "Bob,,Alice")
        if (targets[i].empty())
            continue;

        privmsgOneTarget(
            client,
            targets[i],
            message);
    }
}

void Server::privmsgOneTarget(Client &client,
                              const std::string &target,
                              const std::string &message)
{
    if (target[0] == '#' || target[0] == '&')
    {
        Channel *channel = findChannel(target);

        // 403
        if (!channel)
        {
            sendMessage(client,
                ":ircserv 403 " +
                client.getNickname() +
                " " +
                target +
                " :No such channel\r\n");
            return;
        }

        // 442
        if (!channel->hasMember(&client))
        {
            sendMessage(client,
                ":ircserv 442 " +
                client.getNickname() +
                " " +
                target +
                " :You're not on that channel\r\n");
            return;
        }

        // 404
        // Plus tard :
        // vérifier +m, +b, etc.

        std::string reply =
            buildCommandReply(client,"PRIVMSG",target,message);

        broadcast(*channel, reply, &client);
    }
    else
    {
        Client *receiver = findClient(target);

        // 401
        if (!receiver)
        {
            sendMessage(client,
                ":ircserv 401 " +
                client.getNickname() +
                " " +
                target +
                " :No such nick\r\n");
            return;
        }

        std::string reply =
            buildCommandReply(client,"PRIVMSG",target,message);
        sendMessage(*receiver, reply);
    }
}
