#include "../include/Server.hpp"

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
