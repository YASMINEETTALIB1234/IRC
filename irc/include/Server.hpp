#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <poll.h>//poll() et pollfd()

#include <sys/socket.h>//contient les fonctions de socket
#include <netinet/in.h>//contient sockaddr_in htons() INADDR_ANY
#include <arpa/inet.h>//Elle contient les fonctions liées aux adresses IP(inet_addr(), inet_ntoa())

#include <unistd.h>//close()
#include <cctype>

#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"

class Server
{
private:
    int port;
    std::string password;

    int serverFd;

    std::vector<pollfd> pollFds;
    std::map<int, Client> clients;//Une map fonctionne comme un dictionnaire.(cle=FD, valeur=client)
    std::map<std::string, Channel> channels;

public:
    Server(int port, const std::string &password);
    ~Server();

    //server
    void initServer();
    void run();
    
    //commands server
    void handlePass(Client &client, const std::vector<std::string> &arguments);
    void handleNick(Client &client, const std::vector<std::string> &arguments);
    void handleUser(Client &client, const std::vector<std::string> &arguments);

    //utils server
    void sendWelcome(Client &client);
    void sendRPLWelcome(Client &client);
    void sendRPLYourHost(Client &client);
    void sendRPLCreated(Client &client);
    void sendRPLMyInfo(Client &client);
    bool isValidNickname(const std::string &nickname);
    std::string getClientName(Client &client);
    bool isRegistered(Client &client);


    std::map<std::string, Channel> &getChannels();
    void joinOneChannel(Client &client,
                               const std::string &channelName,
                               const std::string &key);
    void joinManyChannels(Client &client,
                                 const std::vector<std::string> &arguments);

    void joinCommand(Client &client,
                        const std::vector<std::string> &arguments);

    // utils
    std::vector<std::string> split(const std::string&, char);
    bool isValidChannelName(const std::string&);
    Channel *findChannel(const std::string&);
    Channel *createChannel(const std::string&);
    bool isFirstMember(Channel&);
    void joinClient(Channel&, Client&);
    void sendMessage(Client&, const std::string&);
    void broadcast(Channel&, const std::string&);
    void sendTopicReply(Client&, Channel&);
    std::string buildNamesList(Channel&);
    void sendNamesReply(Client&, Channel&);
    //
    Client *findClientInChannel(Channel &channel, const std::string &nick);
    void sendChannelModeIs(Client &client, Channel &channel);
    
    //invite
    Client *findClientByNickname(const std::string &nick);
    void inviteCommand(Client &client, const std::vector<std::string> &arguments);

//topic
    void topicCommand(Client &client, const std::vector<std::string> &arguments);
    //mode
    void modeCommand(Client &client, const std::vector<std::string> &arguments);
    //kick
    void kickCommand(Client &client, const std::vector<std::string> &arguments);
    //quit
    void quitCommand(Client &client, const std::vector<std::string> &arguments);
    void disconnectClient(Client &client, const std::string &reason);


    std::string buildCommandReply(Client &client,
                                      const std::string &command,
                                      const std::string &target,
                                      const std::string &trailing);
};

#endif