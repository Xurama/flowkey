// FlowKey/Server/main.cpp - Application Maître (Windows PC) - Envoi de paquets de test

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

#include "../Core/NetworkManager.hpp"
// NOTE: EventStructs.hpp est inclus par NetworkManager.hpp

using boost::asio::ip::tcp;
// Suppression de 'using namespace FlowKey;' pour éviter l'ambiguïté

const unsigned short FLOWKEY_PORT = 24800; // Port standard

// Classe de test pour la connexion Serveur
class TestServer
{
private:
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    FlowKey::Connection connection_; // Utilisation de FlowKey::Connection
    FlowKey::Connection connection_; // Utilisation de FlowKey::Connection

public:
    TestServer()
        : acceptor_(io_context_, tcp::endpoint(tcp::v4(), FLOWKEY_PORT)),
          connection_(io_context_) {}

    void run()
    {
        std::cout << "Initialisation du Serveur..." << std::endl;
        std::cout << "Serveur FlowKey (Windows) en attente sur le port " << FLOWKEY_PORT << "..." << std::endl;

        try
        {
            // 1. Attente de la connexion client
            std::cout << "Attente de connexion Client..." << std::endl;
            acceptor_.accept(connection_.socket());
            std::cout << "Client connecté depuis: " << connection_.socket().remote_endpoint() << std::endl;

            // 2. Phase de test: Envoi de 5 événements Mouse Move simulés
            FlowKey::MouseEvent test_move; // Préciser FlowKey::MouseEvent
            test_move.deltaX = 100;        // Déplacement de 100 pixels
            test_move.deltaY = 50;

            for (int i = 0; i < 5; ++i)
            {
                std::cout << "Serveur: Envoi du paquet de test Mouse Move #" << i + 1 << std::endl;
                connection_.send_event(FlowKey::EventType::MOUSE_MOVE, &test_move, sizeof(FlowKey::MouseEvent));
                // Petite pause pour simuler le temps réel
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            // Envoi d'un événement clavier simulé
            FlowKey::KeyEvent test_key; // Préciser FlowKey::KeyEvent
            test_key.action = FlowKey::Action::PRESS;
            test_key.modifiers = 0;
            test_key.keyCode = 0x1E; // Code de la touche 'A' sur la plupart des OS
            std::cout << "Serveur: Envoi du paquet de test Key PRESS." << std::endl;
            connection_.send_event(FlowKey::EventType::KEYBOARD, &test_key, sizeof(FlowKey::KeyEvent));
        }
        catch (const boost::system::system_error &e)
        {
            std::cerr << "Erreur Serveur: " << e.what() << std::endl;
        }
    }
};

int main()
{
    try
    {
        // Nécessite l'inclusion de <thread> et <chrono> pour std::this_thread::sleep_for
        std::cout << "Initialisation du Serveur..." << std::endl;
        TestServer server;
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception fatale: " << e.what() << std::endl;
    }
    std::cout << "Serveur terminé." << std::endl;
    return 0;
}