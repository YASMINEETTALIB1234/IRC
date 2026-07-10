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

#include "Client.hpp"

class Server
{
private:
    int port;
    std::string password;

    int serverFd;

    std::vector<pollfd> pollFds;
    std::map<int, Client> clients;//Une map fonctionne comme un dictionnaire.(cle=FD, valeur=client)

public:
    Server(int port, const std::string &password);
    ~Server();

    void initServer();
    void run();

    //void acceptClient();
    //void receiveMessage(int clientFd);
};

#endif