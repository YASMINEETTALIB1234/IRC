#include "../include/Parser.hpp"

#include <sstream>

Parser::Parser()
{
}

Parser::~Parser()
{
}

//Si on ne vide pas les anciennes données, le parser gardera les anciennes valeurs.
void Parser::clear()
{
    command.clear();
    arguments.clear();
}

/*
    Maintenant imaginons ceci
    Tu n'as pas le clavier.
    Tu as juste une chaîne :
    std::string message = "PASS password";
    Comment faire :
    message >> word;
    ❌ Impossible !
    Parce que std::string n'est pas un flux.
    C'est ici qu'intervient stringstream
    Quand tu écris :
    std::stringstream ss(message);
    Tu demandes à C++ :
    Transforme cette chaîne de caractères en un flux, comme std::cin.
    Autrement dit :
    Avant :
    message
    ↓
    "PASS password"
    Après :
    stringstream ss
    ↓
    PASS password
    Maintenant, tu peux utiliser :
    >>
    exactement comme avec std::cin.
    Exemple
    Tu as :
    std::stringstream ss("PASS password");
    Puis :
    std::string word;
    Ensuite :
    ss >> word;
    Que contient word ?
    Réponse :
    PASS
    Pourquoi ?
    Parce que >> lit jusqu'au premier espace.
    Que reste-t-il dans le stream ?
    Après avoir lu :
    PASS
    Le stream est maintenant ici :
    PASS password
        ^
    Il reste :
    password
    Si tu fais encore :

    ss >> word;

    Alors :
    word
    ↓
    password
    Le stream devient :
    PASS password
                ^
    Il n'y a plus rien.
    Si tu fais encore :
    ss >> word;
    Il échoue.
    C'est exactement pour cela qu'on écrit :
    while (ss >> word)
    Le while signifie :
    Continue tant qu'il reste un mot à lire.
    Petit exemple complet
    std::stringstream ss("MODE #42 +i");
    std::string word;
    while (ss >> word)
    {
        std::cout << word << std::endl;
    }
    Affichera :
    MODE
    #42
    +i
    Tu n'as jamais eu besoin de chercher les espaces toi-même.
    stringstream le fait automatiquement.
    Une analogie
    Imagine cette phrase :
    PASS password
    Sans stringstream, c'est une feuille de papier.
    Tu peux la regarder, mais tu ne peux pas lui dire :
    "Donne-moi le premier mot."
    Avec stringstream, cette feuille devient un distributeur de mots.
    À chaque fois que tu fais :
    ss >> word;
    Le distributeur te donne le mot suivant.
    Première fois :
    PASS
    Deuxième fois :
    password
    Troisième fois :
    (rien)
*/

// void Parser::parse(const std::string &message)//message comme "NICK yasmine" etc
// {
//     // Réinitialiser le parser avant une nouvelle commande
//     clear();

//     std::stringstream ss(message);//Transforme cette chaîne de caractères en un flux, comme std::cin
//     std::string word;

//     // Lire la commande (PASS, NICK, USER...)
//     if (!(ss >> command))//(ss >> command qui retourne l'etat du stream) Lire le prochain mot du stream et le mettre dans command.
//         return;

//     // Lire tous les arguments jusqu'a la fin
//     while (ss >> word)
//     {
//         arguments.push_back(word);
//     }
// }

void Parser::parse(const std::string &message)
{
    clear();

    std::stringstream ss(message);
    std::string word;

    // Lire la commande
    if (!(ss >> command))
        return;

    // Lire les paramètres
    while (ss >> word)
    {
        // Si le paramètre commence par ':'
        if (word[0] == ':')
        {
            // Enlever ':'
            word.erase(0, 1);

            std::string rest;
            std::getline(ss, rest);

            word += rest;

            arguments.push_back(word);

            break;
        }

        arguments.push_back(word);
    }
}

std::string Parser::getCommand() const
{
    return command;
}

std::vector<std::string> Parser::getArguments() const
{
    return arguments;
}