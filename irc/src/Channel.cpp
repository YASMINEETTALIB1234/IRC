#include "../include/Channel.hpp"

Channel::Channel()
    : topicSetAt_channel(0),
      inviteOnly_channel(false),
      topicRestricted_channel(false),
      hasKey_channel(false),
      userLimit_channel(0),
      hasLimit_channel(false)
{
}

Channel::Channel(const std::string &name)
    : name_channel(name),
      topicSetAt_channel(0),
      inviteOnly_channel(false),
      topicRestricted_channel(false),
      hasKey_channel(false),
      userLimit_channel(0),
      hasLimit_channel(false)
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

bool Channel::isInviteOnly() const { return inviteOnly_channel; }
void Channel::setInviteOnly(bool value) { inviteOnly_channel = value; }

bool Channel::isTopicRestricted() const { return topicRestricted_channel; }
void Channel::setTopicRestricted(bool value) { topicRestricted_channel = value; }

bool Channel::hasKey() const { return hasKey_channel; }
const std::string &Channel::getKey() const { return key_channel; }
void Channel::setKey(const std::string &key)
{
    key_channel = key;
    hasKey_channel = true;
}
void Channel::removeKey()
{
    key_channel.clear();
    hasKey_channel = false;
}

bool Channel::hasUserLimit() const { return hasLimit_channel; }
size_t Channel::getUserLimit() const { return userLimit_channel; }
void Channel::setUserLimit(size_t limit)
{
    userLimit_channel = limit;
    hasLimit_channel = true;
}
void Channel::removeUserLimit()
{
    userLimit_channel = 0;
    hasLimit_channel = false;
}

void Channel::removeOperator(Client *client)
{
    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i] == client)
        {
            operators.erase(operators.begin() + i); //?
            break;
        }
    }
}

void Channel::addInvite(const std::string &nick)
{
    if (!isInvited(nick))
        invitedNicks_channel.push_back(nick); //?
}

bool Channel::isInvited(const std::string &nick) const
{
    for (size_t i = 0; i < invitedNicks_channel.size(); i++)
    {
        if (invitedNicks_channel[i] == nick)
            return true;
    }
    return false;
}

void Channel::removeInvite(const std::string &nick)
{
    for (size_t i = 0; i < invitedNicks_channel.size(); i++)
    {
        if (invitedNicks_channel[i] == nick)
        {
            invitedNicks_channel.erase(invitedNicks_channel.begin() + i);
            break;
        }
    }
}

std::string Channel::getModeString() const
{
    std::string modes = "+";
    if (inviteOnly_channel) modes += "i";
    if (topicRestricted_channel) modes += "t";
    if (hasKey_channel) modes += "k";
    if (hasLimit_channel) modes += "l";
    return modes;
}