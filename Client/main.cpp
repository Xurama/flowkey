// FlowKey/Client/main.cpp - Application Esclave (MacBook Air) - Réception et Injection

#include <iostream>
#include <boost/asio.hpp>
#include <vector>

#include "../Core/NetworkManager.hpp"
#include "../OS_Specific/macOS/InputInjector.hpp" 

using boost::asio::ip::tcp;

const unsigned short FLOWKEY_PORT = 24800;

class TestClient
{
private:
    boost::asio::io_context io_context_;
    FlowKey::Connection connection_;
    FlowKey::macOS::InputInjector injector_; // Instance de l'injecteur

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

            // 2. Phase de production: Réception et Injection des événements en boucle
            FlowKey::BaseEvent base;
            std::vector<char> payload;

            while (connection_.receive_event(base, payload))
            {
                std::cout << "-----------------------------------" << std::endl;
                
                if (payload.size() != base.size) {
                    std::cerr << "Erreur: Taille de payload incohérente." << std::endl;
                    continue;
                }

                if (base.type == FlowKey::EventType::MOUSE_MOVE && base.size == sizeof(FlowKey::MouseEvent))
                {
                    const FlowKey::MouseEvent *move_event = reinterpret_cast<const FlowKey::MouseEvent *>(payload.data());
                    
                    // INJECTION
                    injector_.injectMouseMove(*move_event);
                    
                    std::cout << "-> MOUSE_MOVE: Injecté X=" << move_event->deltaX
                              << ", Y=" << move_event->deltaY << std::endl;
                }
                else if (base.type == FlowKey::EventType::MOUSE_BUTTON && base.size == sizeof(FlowKey::ButtonEvent))
                {
                    const FlowKey::ButtonEvent *btn_event = reinterpret_cast<const FlowKey::ButtonEvent *>(payload.data());
                    
                    // INJECTION
                    injector_.injectButtonEvent(*btn_event);
                    
                    std::cout << "-> MOUSE_BUTTON: Injecté Action=" << (btn_event->action == FlowKey::Action::PRESS ? "PRESS" : "RELEASE")
                              << ", Code=" << (int)btn_event->buttonCode << std::endl;
                }
                else if (base.type == FlowKey::EventType::KEYBOARD && base.size == sizeof(FlowKey::KeyEvent))
                {
                    const FlowKey::KeyEvent *key_event = reinterpret_cast<const FlowKey::KeyEvent *>(payload.data());
                    
                    // NOTE: Non implémenté dans l'injecteur pour l'instant
                    injector_.injectKeyEvent(*key_event); 
                    
                    std::cout << "-> KEYBOARD: Code=" << key_event->keyCode << " (Non injecté)" << std::endl;
                }
                else
                {
                    std::cout << "-> Type d'événement non géré ou taille invalide. Type=" << static_cast<int>(base.type) << std::endl;
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
    catch (const std::exception &e)aezsq
    {
        std::cerr << "Exception fatale: " << e.what() << std::endl;
    }
    std::cout << "Client terminé." << std::endl;
    return 0;
}