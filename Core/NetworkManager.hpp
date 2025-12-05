// FlowKey/Core/NetworkManager.hpp - Définition de la classe de connexion TCP

#ifndef FLOWKEY_NETWORKMANAGER_HPP
#define FLOWKEY_NETWORKMANAGER_HPP

#include <boost/asio.hpp>
#include "EventStructs.hpp" // Inclut les structures d'événements
#include <vector>
#include <iostream>

namespace FlowKey
{

    using boost::asio::ip::tcp;

    class Connection
    {
    protected:
        boost::asio::io_context &io_context_;
        tcp::socket socket_;

    public:
        Connection(boost::asio::io_context &io_context)
            : io_context_(io_context), socket_(io_context) {}

        virtual ~Connection() = default;

        /**
         * @brief Envoie un paquet (header + payload) via TCP.
         */
        void send_event(EventType type, const void *data, uint8_t size);

        /**
         * @brief Reçoit un paquet (header + payload) via TCP.
         */
        bool receive_event(BaseEvent &base, std::vector<char> &payload_buffer);

        tcp::socket &socket() { return socket_; }
    };

}

#endif // FLOWKEY_NETWORKMANAGER_HPP