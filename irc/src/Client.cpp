#include "../include/Client.hpp"

Client::Client()
    : fd(-1),
      nickname(""),
      username(""),
      authenticated(false)
{
}

Client::Client(int fd)
    : fd(fd),
      nickname(""),
      username(""),
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

void Client::setAuthenticated(bool value)
{
    authenticated = value;
}