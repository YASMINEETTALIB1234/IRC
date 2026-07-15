#include "../include/Server.hpp"

//serverFd(-1) : Mais avant d'appeler : socket() on ne possède aucun FD.
Server::Server(int port, const std::string &password)
    : port(port),
      password(password),
      serverFd(-1)
{
}
//if (serverFd != -1) : Lorsque ton programme quitte :CTRL+C ou return 0; il faut fermer la socket. Sinon le système garde la ressource ouverte.
Server::~Server()
{
    if (serverFd != -1)
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

void Server::handlePass(Client &client, const std::vector<std::string> &arguments)
{
    /*
        Pourquoi passer Client &client par référence (&) ?
        Très bonne question pour anticiper.
        Si on écrivait :
        void handlePass(Client client)
        on travaillerait sur une copie du client.
        Donc :
        client.setAuthenticated(true);
        modifierait seulement la copie.
        Le vrai client dans la map resterait :
        authenticated = false
        En utilisant :
        Client &client
        on modifie directement l'objet stocké dans :
        clients[clientFd]
        C'est exactement ce qu'on veut


        À quoi sert send() ?
        Tu connais déjà :
        recv()
        ↓
        Client --------> Serveur
        Maintenant :
        send()
        fait l'inverse :
        Serveur --------> Client
        Sa syntaxe
        send(
            client.getFd(),
            message.c_str(),
            message.length(),
            0
        );
        Comme recv() :
        recv(fd, buffer, sizeof(buffer), 0);
        mais dans l'autre sens.
        Pourquoi message.c_str() ?
        Tu sais que :
        std::string message = "Welcome";
        est un objet C++.
        Mais :
        send()
        vient du langage C.
        Il ne connaît pas std::string.
        Il veut un :
        const char *
        Donc :
        message.c_str()
        transforme :
        std::string
        ↓
        const char *
        
        Pourquoi message.length() ?
        Parce que send() ne sait pas où la chaîne se termine.
        Il faut lui dire :
        Combien d'octets dois-je envoyer ?
        Donc :
        message.length()

        Pourquoi \r\n et pas seulement \n ?
        En IRC, toutes les lignes se terminent par :
        \r\n
        Ce n'est pas un choix, c'est une règle du protocole IRC (RFC 1459 / RFC 2812).
    */

    if (isRegistered(client))
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
        client.setAuthenticated(false); 
        std::string reply =":ircserv 464 * :Password incorrect\r\n";
        send(client.getFd(), reply.c_str(), reply.length(), 0);
        return;
    }
}



bool Server::isValidNickname(const std::string &nickname)
{
    if (nickname.empty())
        return false;

    
    // Le premier caractère ne peut pas être un chiffre,
    // un '#', un ':', ni un espace.
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
    /*
        D'abord, rappel : c'est quoi clients ?
        Dans Server.hpp, tu as :
        std::map<int, Client> clients;
        Une map est un dictionnaire :
        clé  ---------> valeur
        Chez toi :
        FD ---------> Client
        Par exemple :
        clients
        4  ---------> Client(fd=4, nickname="yasmine")
        5  ---------> Client(fd=5, nickname="ahmed")
        8  ---------> Client(fd=8, nickname="sara")
        Notre objectif
        Un nouveau client écrit :
        NICK ahmed
        Le serveur doit répondre à cette question :
        Est-ce qu'il existe déjà un client dont le nickname est "ahmed" ?
        Pour répondre, on doit regarder tous les clients.
        C'est exactement le rôle de cette boucle.
        Première partie
        clients.begin()
        Que signifie begin() ?
        Il signifie :
        Donne-moi le premier élément de la map.
        Si la map contient :
        4 -> yasmin
        5 -> ahme
        8 -> sar
        Alors 
        clients.begin(
        pointe sur 
        4 -> yasmin
        C'est le début de la map
        Ensuite
        clients.end(
        Attention
        Beaucoup d'étudiants pensent que end() est le dernier élément
        ❌ Faux
        end() signifie 
        La position juste après le dernier élément
        Imagine 
        4 -> yasmin
        5 -> ahmed
        8 -> sara
                ^
            end()
        end() ne contient aucun client.
        C'est juste la limite de la boucle.
        Ensuite
        iterator it
        Tu peux voir un iterator comme un doigt 👇.
        Au début :
        it
        ↓
        4 -> yasmine
        Après
        ++it;
        Le doigt avance.
        it
        ↓
        5 -> ahmed
        Encore :
        ++it;
        ↓
        8 -> sara
        Encore :
        ++it;
        ↓
        end()
        La boucle s'arrête.
        Donc le for
        for (...; it != clients.end(); ++it)
        signifie simplement :
        Tant que je ne suis pas arrivé à la fin, avance d'un client.
        Maintenant la ligne mystérieuse
        it->second
        Pourquoi second ?
        Parce qu'une map stocke des paires (std::pair) :
        clé        valeur
        first      second
        Chez toi :
        4  ---------> Client(...)
        est en réalité :
        first  = 4
        second = Client(...)
        Donc :
        it->first
        ↓
        4
        et
        it->second
        ↓
        Client(...)
        Exemple
        Supposons :
        clients
        4 -> yasmine
        5 -> ahmed
        Premier tour :
        it
        ↓
        4 -> yasmine
        Alors :
        it->first
        ↓
        4
        it->second
        ↓
        Client(fd=4)
        Donc
        it->second.getNickname()
        ↓
        "yasmine"
        Deuxième tour
        it
        ↓
        5 -> ahmed
        Alors :
        it->second.getNickname()
        ↓
        "ahmed"
        Maintenant cette ligne
        if (it->second.getNickname() == arguments[0])
        Supposons que le nouveau client a écrit :
        NICK ahmed
        Donc :
        arguments[0]
        ↓
        "ahmed"
        Premier tour :
        yasmine == ahmed
        ↓
        false
        Deuxième tour :
        ahmed == ahmed
        ↓
        true
        Alors :
        send(...)
        pour dire :
        Nickname already in use
        Résumé visuel
        clients
        4 -> yasmine
        5 -> ahmed
        8 -> sara
        La boucle fait :
        Début
        ↓
        4 -> yasmine
        ↓
        5 -> ahmed
        ↓
        8 -> sara
        ↓
        end()
        À chaque étape, elle demande :
        Est-ce que ce nickname
        est celui demandé ?
        ↓
        Non
        ↓
        Continuer
        ↓
        Oui
        ↓
        Erreur 433
        🎯 Petite question pour vérifier que tu as compris
        Si la map contient :
        clients
        4 -> yasmine
        7 -> ali
        12 -> sara
        et qu'un nouveau client envoie :
        NICK sara
        À quel tour de boucle la condition :
        it->second.getNickname() == arguments[0]


        En bref :
    Pour un **iterator**, on préfère **`++it`** plutôt que **`it++`**.
    * `++it` (**pré-incrémentation**) : avance directement l'iterator. ✅ Plus efficace.
    * `it++` (**post-incrémentation**) : fait une copie de l'ancien iterator, puis avance. ❌ Une copie inutile est créée.
    Exemple :
    ```cpp
    ++it;   // avance directement
    ```
    vs
    ```cpp
    it++;   // copie l'ancien iterator, puis avance
    ```
    Pour une variable simple comme `int i`, il n'y a presque pas de différence :
    ```cpp
    for (int i = 0; i < 10; i++)
    ```
    ou

    ```cpp
    for (int i = 0; i < 10; ++i)a
    ```
    les deux sont équivalents.
    👉 **Mais avec les iterators (`std::map`, `std::vector`, etc.), la convention en C++ est d'utiliser `++it`** car c'est potentiellement plus performant et c'est la bonne pratique.
*/


    /*
    Que devra faire handleNick() ?
    Comme pour PASS, on va créer une fonction dédiée :
    void handleNick(Client &client,
                    const std::vector<std::string> &arguments);
    Elle devra suivre cet ordre :
    1. Vérifier qu'il y a un argument
    Le client écrit :
    NICK
    ↓
    Le serveur répond :
    :ircserv 431 * :No nickname given
    (431 = ERR_NONICKNAMEGIVEN)
    2. Vérifier que le nickname n'est pas déjà utilisé
    Par exemple :
    Client 1 :
    NICK yasmine
    ↓
    accepté ✅
    Client 2 :
    NICK yasmine
    ↓
    refusé
    :ircserv 433 * yasmine :Nickname is already in use
    (433 = ERR_NICKNAMEINUSE)
    3. Sauvegarder le nickname
    client.setNickname(arguments[0]);
    4. Afficher
    std::cout << "Nickname set to "
            << client.getNickname()
            << std::endl;
    Pour le débogage.
    Une nouvelle difficulté
    Pour vérifier si un nickname est déjà utilisé, il faudra parcourir :
    std::map<int, Client> clients;
    Par exemple :
    FD 4  →  nickname = yasmine
    FD 5  →  nickname = ahmed
    FD 6  →  nickname = sara
    Quand un nouveau client demande :
    NICK ahmed
    Le serveur devra parcourir la map et répondre :
    ❌ Ce nickname existe déjà.
    C'est la première fois où ta map<int, Client> va vraiment être utilisée.
*/

std::string Server::getClientName(Client &client)
{
    if (client.getNickname().empty())
        return "*";
    return client.getNickname();
}

std::map<std::string, Channel> &Server::getChannels()
{
    return channels;
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

    // USER <username> <hostname> <servername> :<realname>
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

    // Pour l'instant on ignore :
    // arguments[1] -> hostname
    // arguments[2] -> servername
    // arguments[3] -> realname
}

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

                        Parser parser;
                        parser.parse(buffer);

                        std::vector<std::string> arguments = parser.getArguments();
                        if (parser.getCommand() == "PASS")
                            handlePass(client, arguments);
                        else if (parser.getCommand() == "NICK")
                            handleNick(client, arguments);
                        else if (parser.getCommand() == "USER")
                            handleUser(client, arguments);
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

/*
    Client (nc)
        │
        ▼
    PASS password
        │
        ▼
    recv()
        │
        ▼
    buffer = "PASS password"
        │
        ▼
    Parser::parse()
        │
        ▼
    command = "PASS"
    arguments[0] = "password"
        │
        ▼
    Affichage
*/

