// FlowKey/Core/NetworkManager.cpp - Implémentation des fonctions d'envoi/réception synchrone

#include "NetworkManager.hpp"
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

namespace FlowKey
{

    void Connection::send_event(EventType type, const void *data, uint8_t size)
    {
        if (!socket_.is_open())
        {
            std::cerr << "Erreur: Le socket n'est pas ouvert pour l'envoi." << std::endl;
            return;
        }

        // 1. Préparer le BaseEvent (Header)
        BaseEvent base;
        base.type = type;
        base.size = size;

        // 2. Créer un tampon (buffer) pour l'envoi
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(&base, sizeof(BaseEvent)));

        // Ajouter les données spécifiques (payload)
        if (size > 0 && data != nullptr)
        {
            buffers.push_back(boost::asio::buffer(data, size));
        }

        // 3. Envoyer tout le tampon de manière atomique (TCP)
        try
        {
            boost::asio::write(socket_, buffers);
        }
        catch (const boost::system::system_error &e)
        {
            std::cerr << "Erreur d'envoi réseau: " << e.what() << std::endl;
        }
    }

    bool Connection::receive_event(BaseEvent &base, std::vector<char> &payload_buffer)
    {
        if (!socket_.is_open())
        {
            return false;
        }

        boost::system::error_code error;

        // 1. Lire le BaseEvent (Header)
        size_t header_read = boost::asio::read(socket_,
                                               boost::asio::buffer(&base, sizeof(BaseEvent)),
                                               error);

        if (error == boost::asio::error::eof)
        {
            std::cout << "Connexion fermée par le pair." << std::endl;
            socket_.close();
            return false;
        }
        if (error || header_read != sizeof(BaseEvent))
        {
            std::cerr << "Erreur de lecture de l'en-tête réseau: " << error.message() << std::endl;
            socket_.close();
            return false;
        }

        // 2. Lire le Payload (si size > 0)
        if (base.size > 0)
        {
            payload_buffer.resize(base.size);
            size_t payload_read = boost::asio::read(socket_,
                                                    boost::asio::buffer(payload_buffer.data(), base.size),
                                                    error);

            if (error || payload_read != base.size)
            {
                std::cerr << "Erreur de lecture du payload réseau: " << error.message() << std::endl;
                socket_.close();
                return false;
            }
        }
        else
        {
            payload_buffer.clear();
        }

        return true;
    }

} // namespace FlowKey