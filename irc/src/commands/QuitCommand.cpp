#include "../../include/Server.hpp"
#include <set>

// Rassemble tous les membres des channels partages avec ce client (sans doublons,
// et sans le client lui-meme), puis leur envoie un seul message QUIT chacun.
// Retire aussi le client de tous les channels, et supprime les channels devenus vides.
void Server::disconnectClient(Client &client, const std::string &reason)
{
    std::set<Client*> recipients;

    // 1. Trouver tous les destinataires (membres des channels partages)
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        Channel &channel = it->second;

        if (!channel.hasMember(&client))
            continue;

        std::vector<Client*> &members = channel.getMembers();
        for (size_t i = 0; i < members.size(); i++)
        {
            if (members[i] != &client)
                recipients.insert(members[i]);
        }
    }

    // 2. Construire et envoyer le message QUIT une seule fois par destinataire
    std::string quitMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@" +
                           client.getHost() + " QUIT :" + reason + "\r\n";

    for (std::set<Client*>::iterator it = recipients.begin(); it != recipients.end(); ++it)
        sendMessage(**it, quitMsg);

    // 3. Retirer le client de tous les channels, et supprimer les channels vides
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); )
    {
        Channel &channel = it->second;

        if (channel.hasMember(&client))
            channel.removeMember(&client);

        if (channel.getMembers().empty())
        {
            std::map<std::string, Channel>::iterator toErase = it;
            ++it;
            channels.erase(toErase);
        }
        else
            ++it;
    }

    // 4. Notifier le client lui-meme puis fermer la connexion
    std::string errorMsg = "ERROR :Closing Link: " + client.getNickname() + " (" + reason + ")\r\n";
    sendMessage(client, errorMsg);

    close(client.getFd());
    clients.erase(client.getFd());
}

void Server::quitCommand(Client &client, const std::vector<std::string> &arguments)
{
    std::string reason;

    if (!arguments.empty())
        reason = arguments[0];

    disconnectClient(client, "Quit: " + reason);
}