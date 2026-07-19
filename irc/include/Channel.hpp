#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel
{
private:
    std::string name_channel;

    std::string topic_channel;

    std::vector<Client*> members_channel;

    std::vector<Client*> operators;

    std::string topicSetBy_channel;   // nick de qui a defini le topic actuel
    time_t      topicSetAt_channel;   // timestamp unix de la derniere modification

    //mode
    bool inviteOnly_channel;
    bool topicRestricted_channel;
    bool hasKey_channel;
    std::string key_channel;
    size_t userLimit_channel;
    bool hasLimit_channel;
    std::vector<std::string> invitedNicks_channel;


public:
    Channel();
    Channel(const std::string &name); //Creer un channel si n'existe pas

    const std::string &getName() const;
    ~Channel();

    void addMember(Client *client);

    void removeMember(Client *client);//Retirer un membre (et lui retirer son statut d'operateur si il en avait un)
 
    bool hasMember(Client *client) const;//Vérifier si le client est déjà membre

    std::vector<Client*> &getMembers();//Récupérer tous les membres

    void addOperator(Client *client);//Ajouter un opérateur


    bool isOperator(Client *client) const;//Vérifier si quelqu'un est opérateur (dans topic ,mode..)
    std::vector<Client*> &getOperators();

    //topic
    const std::string &getTopic() const;
    void setTopic(const std::string &topic);
    const std::string &getTopicSetBy() const;
    time_t getTopicSetAt() const;
    void setTopicSetBy(const std::string &nick);
    void setTopicSetAt(time_t when);
    //mode
    // mode i
    bool isInviteOnly() const;
    void setInviteOnly(bool value);
    // mode t
    bool isTopicRestricted() const;
    void setTopicRestricted(bool value);

    // mode k
    bool hasKey() const;
    const std::string &getKey() const;
    void setKey(const std::string &key);
    void removeKey();

    // mode l
    bool hasUserLimit() const;
    size_t getUserLimit() const;
    void setUserLimit(size_t limit);
    void removeUserLimit();

    // mode o (separate from removeMember, which also strips op status)
    void removeOperator(Client *client);

    // invite list, used by INVITE and checked by JOIN when +i
    void addInvite(const std::string &nick);
    bool isInvited(const std::string &nick) const;
    void removeInvite(const std::string &nick);

    // for RPL_CHANNELMODEIS (324)
    std::string getModeString() const;

    
};

#endif