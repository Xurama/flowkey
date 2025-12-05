// FlowKey/Server/main.cpp - Application Maître (Windows PC) - Envoi de paquets de test

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

#include "../Core/NetworkManager.hpp"
// NOTE: EventStructs.hpp est inclus par NetworkManager.hpp

using boost::asio::ip::tcp;

const unsigned short FLOWKEY_PORT = 24800; // Port standard

// Classe de test pour la connexion Serveur
class TestServer
{
private:
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
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

            // 2. Phase de test: Envoi de Mouse Move, Button et Key simulés
            FlowKey::MouseEvent test_move;
            test_move.deltaX = 100; // Déplacement X
            test_move.deltaY = 50;  // Déplacement Y

            FlowKey::ButtonEvent test_button;
            test_button.action = FlowKey::Action::PRESS;
            test_button.buttonCode = 1; // Clic gauche

            FlowKey::KeyEvent test_key;
            test_key.action = FlowKey::Action::PRESS;
            test_key.modifiers = 0;
            test_key.keyCode = 0x1E; // Code de la touche 'A'

            // --- Envoi Séquentiel ---
            for (int i = 0; i < 5; ++i)
            {
                // Envoi de mouvements
                std::cout << "Serveur: Envoi MOUSE MOVE #" << i + 1 << std::endl;
                connection_.send_event(FlowKey::EventType::MOUSE_MOVE, &test_move, sizeof(FlowKey::MouseEvent));
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
            }
            
            // Envoi de clic
            std::cout << "Serveur: Envoi BUTTON PRESS (Gauche)." << std::endl;
            connection_.send_event(FlowKey::EventType::MOUSE_BUTTON, &test_button, sizeof(FlowKey::ButtonEvent));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            test_button.action = FlowKey::Action::RELEASE;
            std::cout << "Serveur: Envoi BUTTON RELEASE (Gauche)." << std::endl;
            connection_.send_event(FlowKey::EventType::MOUSE_BUTTON, &test_button, sizeof(FlowKey::ButtonEvent));

            // Envoi de touche clavier
            std::cout << "Serveur: Envoi KEY PRESS (A)." << std::endl;
            connection_.send_event(FlowKey::EventType::KEYBOARD, &test_key, sizeof(FlowKey::KeyEvent));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            test_key.action = FlowKey::Action::RELEASE;
            std::cout << "Serveur: Envoi KEY RELEASE (A)." << std::endl;
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