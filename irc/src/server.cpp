#include "../include/Server.hpp"

Server::Server(int port, const std::string &password)
    : port(port),
      password(password),
      serverFd(-1) //Mais avant d'appeler : socket() on ne possède aucun FD.
{
}
Server::~Server()
{
    if (serverFd != -1) //Lorsque ton programme quitte :CTRL+C ou return 0; il faut fermer la socket. Sinon le système garde la ressource ouverte.
        close(serverFd);
}

void Server::initServer()
{
    // 1. Créer la socket
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    //socket(type d'address(ipv4), type de communication(tcp Parce que IRC fonctionne sur TCP, pas sur UDP.), Choisis automatiquement le protocole adapté au couple AF_INET + SOCK_STREAM.)
    if (serverFd == -1)
    {
        std::cerr << "Error: socket creation failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2. Permet de réutiliser le port immédiatement après la fermeture du serveur
    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        //&opt → où se trouvent les données ;
        //sizeof(opt) → combien d'octets il doit lire (ici 4).
        std::cerr << "Error: setsockopt failed." << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // 3. Préparer l'adresse du serveur
    sockaddr_in serverAddress;

    //Je suis un serveur IPv4, j'écoute sur le port _port et j'accepte les connexions sur toutes les interfaces réseau de cette machine.
    serverAddress.sin_family = AF_INET;//ipv4
    serverAddress.sin_addr.s_addr = INADDR_ANY;//Écoute sur toutes les interfaces réseau disponibles.(si on l'utilise pas et on ecrit par exemple 127.0.0.1 ici Le serveur écouterait uniquement sur localhost et donc pas d'acces a les autres machines)
    serverAddress.sin_port = htons(port);

    // 4. Associer la socket à cette adresse
    /*
        int bind(
            int sockfd, listning socket
            const struct sockaddr *addr,
            socklen_t addrlen, Combien d'octets contient cette structure? pour par exemple ipv4 contient 16 octet
        );
    */
    if (bind(serverFd,
            reinterpret_cast<sockaddr *>(&serverAddress),
            sizeof(serverAddress)) < 0)
    {
        std::cerr << "Error: bind failed." << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // 5. Mettre le serveur en attente de connexions
    //listen(fd, SOMAXCONN); SOMAXCONN --> Combien de demandes de connexion peuvent attendre dans cette file.
    if (listen(serverFd, SOMAXCONN) < 0)
    {
        std::cerr << "Error: listen failed." << std::endl;
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    // 6. Ajouter la socket du serveur dans poll()
    /*
        struct pollfd
        {
            int   fd;
            short events;  Quels événements veux-tu surveiller ? ex: POLLIN Préviens-moi lorsqu'il y a des données à lire ou Préviens-moi lorsqu'un client veut se connecter.
            short revents; howa f lawel kikoun 0 o appres 7na matalan fach khtarina events=POLLIN hna systeme ki7ot f revents=POLLIN
        };
    */
    pollfd serverPoll;

    serverPoll.fd = serverFd;
    serverPoll.events = POLLIN;
    serverPoll.revents = 0;

    //pollFds est juste une liste de toutes les sockets
    pollFds.push_back(serverPoll);//push_back : Ajouter un nouvel élément à la fin du vecteur.

    std::cout << "Server started on port " << port << std::endl;
}


/*
    dans run() Le serveur ne sait rien que : 
            FD4
            ↓
        Qui est-il ?
        Nickname ?
        Username ?
        PASS ?
*/


void Server::run()
{
    while (true)
    {
        // Attendre qu'un événement arrive
        /*
            int poll(
            struct pollfd fds[],
            nfds_t nfds, Combien de pollfd dois-je parcourir ?
            int timeout, Combien de temps poll() doit attendre ?, 
            si -1: Attends indéfiniment jusqu'à ce qu'un événement arrive., 
            si 0 Le système répond immédiatement. Même si aucun client n'a envoyé de message.
            si non attend selon le timout.
            );
        */
       //poll() return Le nombre de File Descriptors qui possèdent un événement.
       //a chaque fois ready incremente selon le nombre de socket qui possèdent un événement.
       //Lorsque poll() se réveille, il met à jour le champ revents de chaque pollfd.
       //Même si poll() indique combien de sockets ont un événement (ready), il ne précise pas lesquelles.
        int ready = poll(pollFds.data(), pollFds.size(), -1);
        if (ready < 0)
        {
            std::cerr << "Error: poll failed." << std::endl;
            break;
        }

        // Parcourir toutes les sockets surveillées
        //Parcourir toutes les sockets surveillées par poll() afin d'identifier celles dont revents indique qu'un événement s'est produit.
        /*
            Le serveur reste actif grâce à while(true).
            poll() surveille toutes les sockets et attend un événement.
            Si une erreur survient, le serveur l'affiche et s'arrête.
            Sinon, le serveur parcourt toutes les sockets pour découvrir lesquelles ont déclenché un événement
        */

       /*
        _pollFds
            Index 0 → FD3 (Serveur)
                        │
                        └── accept()
            Index 1 → FD4 (Client A)
                        │
                        └── recv()
            Index 2 → FD5 (Client B)
                        │
                        └── recv()
            Index 3 → FD6 (Client C)
                        │
                        └── recv()
       */
        for (size_t i = 0; i < pollFds.size(); i++)
        {
            // Vérifier si cette socket possède un événement
            if (pollFds[i].revents & POLLIN)
            {
                // ===============================
                // Cas 1 : Nouvelle connexion
                // ===============================
                //Listening Socket
                //Cette socket a une seule mission :
                //Accepter l es nouvelles connexions.
                if (pollFds[i].fd == serverFd)
                {
                    sockaddr_in clientAddress;
                    socklen_t clientLen = sizeof(clientAddress);

                    /*
                        Pourquoi accept() demande-t-il la taille (clientLen)?
                        Imagine que le noyau doive copier les informations du client dans clientAddress.
                        Il se demande :
                        Combien d'octets puis-je écrire ?

                        Pourquoi une variable et pas directement sizeof(clientAddress) ?
                        Parce que accept() ne veut pas un nombre.
                        Il veut l'adresse d'une variable.

                        Pour bind(), c'est toi qui remplis serverAddress, puis tu la donnes au système.
                        Pour accept(), c'est le système qui remplit clientAddress et te la rend.
                    */
                   
                    int clientFd = accept(
                        serverFd,
                        reinterpret_cast<sockaddr*>(&clientAddress),
                        &clientLen
                    );

                    //si accept() Le File Descriptor (FD) de la nouvelle socket créée pour le client.
                    if (clientFd < 0)
                    {
                        std::cerr << "Accept failed." << std::endl;
                        //Ignore ce tour de boucle et passe directement au suivant.(continue)
                        continue;
                    }

                    clients[clientFd] = Client(clientFd);

                /*
                    struct pollfd
                    {
                        int fd;
                        short events;
                        short revents;
                    };
                */
                    //Pourquoi crée-t-on une nouvelle structure pollfd ?
                    //Parce que poll() ne surveille que les sockets présentes dans _pollFds
                    pollfd newClient;
                    newClient.fd = clientFd;
                    //Préviens-moi lorsque ce client enverra des données
                    newClient.events = POLLIN;
                    //Parce que le client vient juste de se connecter.
                    //Plus tard, lorsque poll() sera appelé, le système remplira automatiquement revents
                    /*
                        Par exemple :
                        Avant poll() :
                        FD4
                        events = POLLIN
                        revents = 0
                        Le client envoie :
                        NICK Yasmine
                        Après poll() :
                        FD4
                        events = POLLIN
                        revents = POLLIN
                        C'est le système qui change revents
                    */
                    newClient.revents = 0;

                    pollFds.push_back(newClient);

                    std::cout << "New client connected. FD = "
                              << clientFd << std::endl;
                }

                // ===============================
                // Cas 2 : Message d'un client
                // ===============================
                else
                {
                    Client &client = clients[pollFds[i].fd];

                    //recv() reçoit des données envoyées par le client.
                    //Mais où va-t-il les mettre ?
                    //512 limit impose par le RFC de IRC 
                    char buffer[512];

                    //Lire les données envoyées par le client et les copier dans buffer
                    //bytes = le nombre de caractere envoyer dans un msg comme(PASS abc = 8 caractere)
                    //bytes = Lit les données envoyées par le client, les place dans buffer et retourne le nombre d'octets lus, stocké dans bytes.
                    int bytes = recv(
                        //Lis les données envoyées par le client dont la socket est FD5 (Bob).
                        //on ne fait jamais recv(_serverFd,...) Parce que _serverFd est la Listening Socket. elle sert uniquement a accept
                        pollFds[i].fd,//C'est la socket sur laquelle on veut lire.
                        buffer,//Nombre maximal d'octets à lire
                        sizeof(buffer) - 1,//Combien d'octets maximum veux-tu lire ?
                        0//Lecture normale, sans option particulière.(il existe bcp d'option)
                    );

                    if (bytes <= 0)
                    {
                        /*
                            close(_pollFds[i].fd);
                            Pourquoi ?
                            Le client est parti.
                            Sa socket n'est plus utile.
                            On demande donc au système :
                            Ferme cette socket et libère les ressources.
                        */
                        close(pollFds[i].fd);
                        clients.erase(pollFds[i].fd);
                        //pollFds.begin() itérateur qui pointe vers premier element
                        pollFds.erase(pollFds.begin() + i);

                        std::cout << "Client disconnected."
                                  << std::endl;
                        //on ne supprime jamais index 0 car :
                        //0 est toujours la Listening Socket.
                        //Les clients commencent à l'indice 1.

                        i--;
                    }
                    else
                    {
                        buffer[bytes] = '\0';

                        std::cout << "Received : "
                                  << buffer
                                  << std::endl;
                    }
                }
            }
        }
    }
}

/*
    Quand un message arrive :
    recv(...)
    Tu connais seulement :
    clientFd = 4;
    Comment retrouver l'objet Client correspondant ?
    C'est exactement pour cela qu'on utilise :
    std::map<int, Client> _clients
*/