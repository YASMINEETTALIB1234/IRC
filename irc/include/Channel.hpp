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


    const std::string &getTopic() const;

//apres
    bool isOperator(Client *client) const;//Vérifier si quelqu'un est opérateur (dans topic ,mode..)
    void setTopic(const std::string &topic);
    std::vector<Client*> &getOperators();

};

#endif