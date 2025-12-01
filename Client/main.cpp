// FlowKey/Client/main.cpp - Application Esclave (MacBook Air)

#include <iostream>
#include <boost/asio.hpp>
#include "../Core/NetworkManager.hpp"

// L'inclusion de EventStructs.hpp est maintenant gérée par NetworkManager.hpp
// #include "../Core/EventStructs.hpp"

using boost::asio::ip::tcp;

const unsigned short FLOWKEY_PORT = 24800;

class TestClient
{
private:
    boost::asio::io_context io_context_;
    FlowKey::Connection connection_;

public:
    TestClient() : connection_(io_context_) {}

    void run(const std::string &server_ip)
    {
        std::cout << "Client FlowKey (macOS) tentant la connexion à " << server_ip << ":" << FLOWKEY_PORT << std::endl;

        try
        {
            // 1. Résolution de l'adresse et connexion
            tcp::resolver resolver(io_context_);
            boost::asio::connect(connection_.socket(), resolver.resolve(server_ip, std::to_string(FLOWKEY_PORT)));
            std::cout << "Connexion établie au serveur." << std::endl;

            // 2. Phase de test: Réception des événements en boucle
            FlowKey::BaseEvent base;
            std::vector<char> payload;

            while (connection_.receive_event(base, payload))
            {
                std::cout << "-----------------------------------" << std::endl;
                std::cout << "Client: Paquet reçu! Type: " << static_cast<int>(base.type)
                          << ", Taille payload: " << static_cast<int>(base.size) << " octets." << std::endl;

                if (base.type == FlowKey::EventType::MOUSE_MOVE && base.size == sizeof(FlowKey::MouseEvent))
                {
                    // Désérialisation pour vérification
                    const FlowKey::MouseEvent *move_event = reinterpret_cast<const FlowKey::MouseEvent *>(payload.data());
                    std::cout << "  -> MOUSE_MOVE: Déplacement X=" << move_event->deltaX
                              << ", Y=" << move_event->deltaY << std::endl;
                }
                else if (base.type == FlowKey::EventType::KEYBOARD && base.size == sizeof(FlowKey::KeyEvent))
                {
                    const FlowKey::KeyEvent *key_event = reinterpret_cast<const FlowKey::KeyEvent *>(payload.data());
                    std::cout << "  -> KEYBOARD: Action=" << (key_event->action == FlowKey::Action::PRESS ? "PRESS" : "RELEASE")
                              << ", KeyCode=" << key_event->keyCode << std::endl;
                }
                else
                {
                    std::cout << "  -> Type d'événement non reconnu ou taille invalide." << std::endl;
                }
            }
        }
        catch (const boost::system::system_error &e)
        {
            std::cerr << "Erreur Client: " << e.what() << std::endl;
        }
    }
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Utilisation: " << argv[0] << " <adresse_ip_serveur>" << std::endl;
        return 1;
    }

    try
    {
        TestClient client;
        client.run(argv[1]);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception fatale: " << e.what() << std::endl;
    }
    std::cout << "Client terminé." << std::endl;
    return 0;
}