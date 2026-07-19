#include "../include/Server.hpp"
#include "../include/Channel.hpp"

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
                        disconnectClient(client, "Connection closed");   // handles channels/broadcast/fd close/erase
                        pollFds.erase(pollFds.begin() + i);
                        i--;
                        std::cout << "Client disconnected." << std::endl;
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
                        else if (parser.getCommand() == "JOIN")
                            joinCommand(client, arguments);
                        else if (parser.getCommand() == "TOPIC")
                            topicCommand(client, arguments);
                        else if (parser.getCommand() == "PRIVMSG")
                            privmsgCommand(client, arguments);
                        else if (parser.getCommand() == "MODE")
                            modeCommand(client, arguments);
                        else if (parser.getCommand() == "INVITE")
                            inviteCommand(client, arguments);
                        else if (parser.getCommand() == "KICK")
                            kickCommand(client, arguments);
                        else if (parser.getCommand() == "QUIT")
                            quitCommand(client, arguments);
                        else if (parser.getCommand() == "PART")
                            partCommand(client, arguments);
                        else
                        {
                            std::string reply = ":ircserv 421 " + parser.getCommand() + " :Unknown command\r\n";
                            send(
                                client.getFd(),
                                reply.c_str(),
                                reply.length(),
                                0
                            );
                        }
                        // If the handler we just called disconnected this client
                        // (currently only quitCommand -> disconnectClient does this),
                        // the fd was already closed and erased from `clients`.
                        // We must also drop it from pollFds so poll() doesn't
                        // watch a dead fd on the next iteration.
                        if (clients.find(pollFds[i].fd) == clients.end())
                        {
                            pollFds.erase(pollFds.begin() + i);
                            i--;
                        }
                    }
                }
            }
        }
    }
}
