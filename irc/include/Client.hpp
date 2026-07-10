#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
    int         fd;//numero de socket de client 
    std::string nickname;//nick name de client
    std::string username;//username de client
    bool        authenticated;//si il est authentifie(PASS)

public:
    Client();
    Client(int fd);
    ~Client();

    int getFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    bool isAuthenticated() const;

    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setAuthenticated(bool value);
};

#endif