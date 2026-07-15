#include "../include/Client.hpp"

Client::Client()
    : fd(-1),
      nickname(""),
      username(""),
      host("localhost"),
      authenticated(false)
{
}

Client::Client(int fd)
    : fd(fd),
      nickname(""),
      username(""),
      host("localhost"),
      authenticated(false)
{
}

Client::~Client()
{
}

int Client::getFd() const
{
    return fd;
}

std::string Client::getNickname() const
{
    return nickname;
}

std::string Client::getUsername() const
{
    return username;
}

std::string Client::getHost() const
{
    return host;
}

bool Client::isAuthenticated() const
{
    return authenticated;
}

void Client::setNickname(const std::string& newnickname)
{
    nickname = newnickname;
}

void Client::setUsername(const std::string& newusername)
{
    username = newusername;
}

void Client::setHost(const std::string& newhost)
{
    host = newhost;
}

void Client::setAuthenticated(bool value)
{
    authenticated = value;
}