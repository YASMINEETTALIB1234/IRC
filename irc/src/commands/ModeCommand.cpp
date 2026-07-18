#include "../../include/Server.hpp"
#include <cstdlib>

// Trouve un client par nickname parmi les membres du channel
Client *Server::findClientInChannel(Channel &channel, const std::string &nick)
{
    std::vector<Client*> &members = channel.getMembers();

    for (size_t i = 0; i < members.size(); i++)
    {
        if (members[i]->getNickname() == nick)
            return members[i];
    }
    return NULL;
}

void Server::sendChannelModeIs(Client &client, Channel &channel)
{
    std::string reply = ":ircserv 324 " + client.getNickname() + " " +
                         channel.getName() + " " + channel.getModeString() + "\r\n";
    sendMessage(client, reply);
}

void Server::modeCommand(Client &client, const std::vector<std::string> &arguments)
{
    if (arguments.empty())
    {
        sendMessage(client, ":ircserv 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }

    std::string target = arguments[0];

    if (target[0] != '#' && target[0] != '&')
    {
        sendMessage(client, ":ircserv 502 " + client.getNickname() + " :Cannot change mode for other users\r\n");
        return;
    }

    Channel *channel = findChannel(target);
    if (!channel)
    {
        sendMessage(client, ":ircserv 403 " + client.getNickname() + " " + target + " :No such channel\r\n");
        return;
    }

    if (!channel->hasMember(&client))
    {
        sendMessage(client, ":ircserv 442 " + client.getNickname() + " " + target + " :You're not on that channel\r\n");
        return;
    }

    // MODE #chan   -> just query current modes
    if (arguments.size() == 1)
    {
        sendChannelModeIs(client, *channel);
        return;
    }

    if (!channel->isOperator(&client))
    {
        sendMessage(client, ":ircserv 482 " + client.getNickname() + " " + target + " :You're not channel operator\r\n");
        return;
    }

    std::string modestring = arguments[1];
    size_t argIndex = 2;
    char sign = '+';

    std::string appliedModes;
    std::string appliedParams;

    for (size_t i = 0; i < modestring.size(); i++)
    {
        char c = modestring[i];

        if (c == '+' || c == '-')
        {
            sign = c;
            continue;
        }

        switch (c)
        {
            case 'i':
                channel->setInviteOnly(sign == '+');
                appliedModes += sign;
                appliedModes += c;
                break;

            case 't':
                channel->setTopicRestricted(sign == '+');
                appliedModes += sign;
                appliedModes += c;
                break;

            case 'k':
                if (sign == '+')
                {
                    if (argIndex >= arguments.size())
                    {
                        sendMessage(client, ":ircserv 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                        break;
                    }
                    channel->setKey(arguments[argIndex]);
                    appliedModes += sign;
                    appliedModes += c;
                    appliedParams += " " + arguments[argIndex];
                    argIndex++;
                }
                else
                {
                    channel->removeKey();
                    appliedModes += sign;
                    appliedModes += c;
                }
                break;

            case 'l':
                if (sign == '+')
                {
                    if (argIndex >= arguments.size())
                    {
                        sendMessage(client, ":ircserv 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                        break;
                    }

                    long limit = std::atol(arguments[argIndex].c_str());
                    if (limit <= 0)
                    {
                        argIndex++;
                        break; // invalid limit, silently skip
                    }

                    channel->setUserLimit(static_cast<size_t>(limit));
                    appliedModes += sign;
                    appliedModes += c;
                    appliedParams += " " + arguments[argIndex];
                    argIndex++;
                }
                else
                {
                    channel->removeUserLimit();
                    appliedModes += sign;
                    appliedModes += c;
                }
                break;

            case 'o':
            {
                if (argIndex >= arguments.size())
                {
                    sendMessage(client, ":ircserv 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                    break;
                }

                std::string nick = arguments[argIndex];
                argIndex++;

                Client *targetClient = findClientInChannel(*channel, nick);
                if (!targetClient)
                {
                    sendMessage(client, ":ircserv 401 " + client.getNickname() + " " + nick + " :No such nick/channel\r\n");
                    break;
                }

                if (sign == '+')
                    channel->addOperator(targetClient);
                else
                    channel->removeOperator(targetClient);

                appliedModes += sign;
                appliedModes += c;
                appliedParams += " " + nick;
                break;
            }

            default:
                sendMessage(client, ":ircserv 472 " + client.getNickname() + " " +
                                    std::string(1, c) + " :is unknown mode char to me\r\n");
                break;
        }
    }

    if (!appliedModes.empty())
    {
        std::string fullMessage = ":" + client.getNickname() + "!" +
                                   client.getUsername() + "@" + client.getHost() +
                                   " MODE " + channel->getName() + " " + appliedModes + appliedParams + "\r\n";
        broadcast(*channel, fullMessage);
    }
}