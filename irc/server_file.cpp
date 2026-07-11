#include "include/Server.hpp"

Server::Server(int port, const std::string &password)
    : port(port),
      password(password),
      serverFd(-1) 
{
}

Server::~Server()
{
    if (serverFd != -1)
        close(serverFd);
}

void Server::initServer()
{
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == -1)
    {
        std::cerr << "Error: socket creation failed." << std::endl;
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Error: setsockopt failed." << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverFd,
            reinterpret_cast<sockaddr *>(&serverAddress),
            sizeof(serverAddress)) < 0)
    {
        std::cerr << "Error: bind failed." << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }
    if (listen(serverFd, SOMAXCONN) < 0)
    {
        std::cerr << "Error: listen failed." << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    pollfd serverPoll;
    serverPoll.fd = serverFd;
    serverPoll.events = POLLIN;
    serverPoll.revents = 0;
    pollFds.push_back(serverPoll);

    std::cout << "Server started on port " << port << std::endl;
}

void Server::handlePass(Client &client, const std::vector<std::string> &arguments)
{
    if (client.isAuthenticated())
    {
        std::string reply = ":ircserv 462 " + getClientName(client) + " :You may not reregister\r\n";
        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );
        return;
    }

    if (arguments.empty())
    {
        std::string reply = ":ircserv 461 * PASS :Not enough parameters\r\n";
        send(client.getFd(), reply.c_str(), reply.length(), 0);
        return;
    }
    if (arguments[0] == password)
    {
        client.setAuthenticated(true);
        std::cout << "Client authenticated successfully." << std::endl;
    }
    else
    {
        std::string reply =":ircserv 464 * :Password incorrect\r\n";
        send(client.getFd(), reply.c_str(), reply.length(), 0);
        return;
    }
}

bool Server::isValidNickname(const std::string &nickname)
{
    if (nickname.empty())
        return false;

    if (std::isdigit(nickname[0]) || nickname[0] == '#' || nickname[0] == ':' || nickname[0] == ' ')
        return false;

    for (size_t i = 0; i < nickname.size(); i++)
    {
        char c = nickname[i];
        if (std::isalnum(c))
            continue;
        if (c == '[' || c == ']' || c == '{' || c == '}' || c == '\\' || c == '|')
            continue;
        return false;
    }
    return true;
}

std::string Server::getClientName(Client &client)
{
    if (client.getNickname().empty())
        return "*";
    return client.getNickname();
}

void Server::handleNick(Client &client, const std::vector<std::string> &arguments)
{

    if (!client.isAuthenticated())
    {
        std::string reply = ":ircserv 451 " + getClientName(client) + " :You have not registered\r\n";
        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );
        return;
    }
    if (arguments.empty())
    {
        std::string reply = ":ircserv 431 * :No nickname given\r\n";
        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0);
        return;
    }
    else if (arguments.size() > 1)
    {
        std::string target = getClientName(client);
        std::string reply = ":ircserv 432 " + target + " " + arguments[0] + " :Erroneous nickname\r\n";
        send(client.getFd(),
            reply.c_str(),
            reply.length(),
            0);

        return;
    }
    std::string nickname = arguments[0];
    if (!isValidNickname(nickname))
    {
        std::string target = getClientName(client);
        std::string reply = ":ircserv 432 " + target + " " + nickname + " :Erroneous nickname\r\n";
        send(client.getFd(),
             reply.c_str(),
             reply.length(),
             0);
        return ;
    }
    for (std::map<int, Client>::iterator it = clients.begin();
         it != clients.end();
         ++it)
    {
        if (it->second.getNickname() == arguments[0])
        {
            std::string reply = ":ircserv 433 * " + arguments[0] + " :Nickname is already in use\r\n";
            send(
                client.getFd(),
                reply.c_str(),
                reply.length(),
                0
            );
            return;
        }
    }
    client.setNickname(arguments[0]);
    std::cout << "Nickname set to : "
              << client.getNickname()
              << std::endl;
}


void Server::sendWelcome(Client &client)
{
    std::string reply =
        ":ircserv 001 " + client.getNickname() + " :Welcome to the ft_irc Network " + client.getNickname() + "\r\n";
    send(
        client.getFd(),
        reply.c_str(),
        reply.length(),
        0
    );
}

bool Server::isRegistered(Client &client)
{
    if (!client.isAuthenticated())
        return false;

    if (client.getNickname().empty())
        return false;

    if (client.getUsername().empty())
        return false;

    return true;
}

void Server::handleUser(Client &client, const std::vector<std::string> &arguments)
{
    if (!client.isAuthenticated())
    {
        std::string reply =
            ":ircserv 451 " + getClientName(client) +
            " :You have not registered\r\n";

        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );

        return;
    }

    if (!client.getUsername().empty())
    {
        std::string reply = ":ircserv 462 " + getClientName(client) + " :You may not reregister\r\n";
        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );
        return;
    }
    if (arguments.size() < 4)
    {
        std::string reply =
            ":ircserv 461 USER :Not enough parameters\r\n";

        send(
            client.getFd(),
            reply.c_str(),
            reply.length(),
            0
        );

        return;
    }

    client.setUsername(arguments[0]);

    std::cout << "Username set to : "
              << client.getUsername()
              << std::endl;

    if (isRegistered(client))
    {
        sendWelcome(client);

        std::cout << "Client successfully registered."
                << std::endl;
    }
}

void Server::run()
{
    while (true)
    {
        int ready = poll(pollFds.data(), pollFds.size(), -1);
        if (ready < 0)
        {
            std::cerr << "Error: poll failed." << std::endl;
            break;
        }
      
        for (size_t i = 0; i < pollFds.size(); i++)
        {
            if (pollFds[i].revents & POLLIN)
            {
                if (pollFds[i].fd == serverFd)
                {
                    sockaddr_in clientAddress;
                    socklen_t clientLen = sizeof(clientAddress);

                    int clientFd = accept(
                        serverFd,
                        reinterpret_cast<sockaddr*>(&clientAddress),
                        &clientLen
                    );

                    if (clientFd < 0)
                    {
                        std::cerr << "Accept failed." << std::endl;
                        continue;
                    }

                    clients[clientFd] = Client(clientFd);
                    pollfd newClient;
                    newClient.fd = clientFd;
                    newClient.events = POLLIN;
                    newClient.revents = 0;

                    pollFds.push_back(newClient);

                    std::cout << "New client connected. FD = "
                              << clientFd << std::endl;
                }
                else
                {
                    Client &client = clients[pollFds[i].fd];

                    char buffer[512];
                    int bytes = recv(
                        pollFds[i].fd,
                        buffer,
                        sizeof(buffer) - 1,
                        0
                    );

                    if (bytes <= 0)
                    {
                        close(pollFds[i].fd);
                        clients.erase(pollFds[i].fd);
                        pollFds.erase(pollFds.begin() + i);

                        std::cout << "Client disconnected."
                                  << std::endl;
                        i--;
                    }
                    else
                    {
                        buffer[bytes] = '\0';

                        Parser parser;
                        parser.parse(buffer);

                        std::vector<std::string> arguments = parser.getArguments();
                        if (parser.getCommand() == "PASS")
                            handlePass(client, arguments);
                        else if (parser.getCommand() == "NICK")
                            handleNick(client, arguments);
                        else if (parser.getCommand() == "USER")
                            handleUser(client, arguments);
                    }
                }
            }
        }
    }
}
