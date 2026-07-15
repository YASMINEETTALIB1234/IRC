#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int         fd;//numero de socket de client 
    std::string nickname;//nick name de client
    std::string username;//username de client
    std::string host;//adresse IP/hostname reelle du client (recuperee a la connexion via accept())
    bool        authenticated;//si il est authentifie(PASS)

public:
    Client();
    Client(int fd);
    ~Client();

    int getFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getHost() const;
    bool isAuthenticated() const;

    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setHost(const std::string& host);
    void setAuthenticated(bool value);
};

#endif