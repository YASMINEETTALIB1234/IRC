#include "../include/Channel.hpp"

Channel::Channel()
    : topicSetAt_channel(0)
{
}

Channel::Channel(const std::string &name)
    : name_channel(name),
      topicSetAt_channel(0)
{
}

Channel::~Channel()
{
}

const std::string &Channel::getName() const
{
    return name_channel;
}

const std::string &Channel::getTopic() const
{
    return topic_channel;
}

void Channel::setTopic(const std::string &topic)
{
    topic_channel = topic;
}

void Channel::addMember(Client *client)
{
    members_channel.push_back(client);
}

std::vector<Client*> &Channel::getMembers()
{
    return members_channel;
}

void Channel::removeMember(Client *client)
{
    for (size_t i = 0; i < members_channel.size(); i++)
    {
        if (members_channel[i] == client)
        {
            members_channel.erase(members_channel.begin() + i);
            break;
        }
    }

    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i] == client)
        {
            operators.erase(operators.begin() + i);
            break;
        }
    }
}

void Channel::addOperator(Client *client)
{
    if (!isOperator(client))
        operators.push_back(client);
}

bool Channel::hasMember(Client *client) const
{
    for (size_t i = 0; i < members_channel.size(); i++)
    {
        if (members_channel[i] == client)
            return true;
    }

    return false;
}

bool Channel::isOperator(Client *client) const
{
    for (size_t i = 0; i < operators.size(); ++i)
    {
        if (operators[i] == client)
            return true;
    }

    return false;
}

std::vector<Client*> &Channel::getOperators()
{
    return operators;
}


const std::string &Channel::getTopicSetBy() const
{
    return topicSetBy_channel;
}

time_t Channel::getTopicSetAt() const
{
    return topicSetAt_channel;
}

void Channel::setTopicSetBy(const std::string &nick)
{
    topicSetBy_channel = nick;
}

void Channel::setTopicSetAt(time_t when)
{
    topicSetAt_channel = when;
}
