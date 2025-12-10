// FlowKey/Server/main.cpp - Application Maître (Windows PC) - Boucle d'Interception d'Entrée

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

#include "../Core/NetworkManager.hpp"
#include "../OS_Specific/Windows/InputInterceptor.hpp" 

using boost::asio::ip::tcp;

const unsigned short FLOWKEY_PORT = 24800; 

// Pointeur statique pour accéder à la connexion réseau depuis l'interceptor
static FlowKey::Connection* g_connection = nullptr;

/**
 * @brief Fonction de callback appelée par l'InputInterceptor lorsqu'un événement est capturé.
 */
void handle_captured_event(FlowKey::EventType type, const void* data, uint8_t size) {
    if (g_connection && g_connection->socket().is_open()) {
        g_connection->send_event(type, data, size);
    }
}

class ServerApp {
private:
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    FlowKey::Connection connection_;
    FlowKey::Windows::InputInterceptor interceptor_;

public:
    ServerApp()
        : acceptor_(io_context_, tcp::endpoint(tcp::v4(), FLOWKEY_PORT)),
          connection_(io_context_),
          interceptor_(handle_captured_event) // Passe le callback à l'intercepteur
    {
        g_connection = &connection_; // Initialise le pointeur statique
    }

    void run() {
        std::cout << "Initialisation du Serveur d'Interception..." << std::endl;
        std::cout << "Serveur FlowKey (Windows) en attente sur le port " << FLOWKEY_PORT << "..." << std::endl;

        try {
            // 1. Attente de la connexion client
            std::cout << "Attente de connexion Client..." << std::endl;
            // Exécuter l'accept() sur un thread séparé pour ne pas bloquer la boucle d'événements
            // NOTE: Pour simplifier le PoC, nous restons synchrone et bloquons l'UI jusqu'à la connexion.
            acceptor_.accept(connection_.socket());
            std::cout << "Client connecté depuis: " << connection_.socket().remote_endpoint() << std::endl;
            std::cout << "--- Démarrage de l'Interception (Mouvement de souris vers la droite pour basculer) ---" << std::endl;
            std::cout << "Pour le moment, les clics et touches ne seront envoyés que lorsque le contrôle est basculé (souris au bord droit)." << std::endl;

            // 2. Démarrer la boucle de messages Windows (bloquant)
            interceptor_.startEventLoop(); 

        } catch (const boost::system::system_error& e) {
            std::cerr << "Erreur Serveur: " << e.what() << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Erreur d'interception: " << e.what() << std::endl;
        }
    }
};

int main() {
    try {
        ServerApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception fatale: " << e.what() << std::endl;
    }
    std::cout << "Serveur terminé (Fermeture manuelle)." << std::endl;
    return 0;
}