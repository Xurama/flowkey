// FlowKey/Core/EventStructs.hpp - Structures de données pour la communication réseau

#ifndef FLOWKEY_EVENTSTRUCTS_HPP
#define FLOWKEY_EVENTSTRUCTS_HPP

#include <cstdint>

// Garantir que les structures ont une taille et un alignement cohérents
// sur toutes les plateformes (Windows et macOS)
#pragma pack(push, 1)

namespace FlowKey
{

    enum class EventType : uint8_t
    {
        MOUSE_MOVE = 0x01,
        MOUSE_BUTTON = 0x02,
        KEYBOARD = 0x03,

        CONTROL_SWITCH_SCREEN = 0x10,
        CONTROL_KEEP_ALIVE = 0x11
    };

    enum class Action : uint8_t
    {
        PRESS = 0x01,
        RELEASE = 0x02,
        SCROLL_UP = 0x03,
        SCROLL_DOWN = 0x04
    };

    // Structure de base de tous les événements (Header)
    struct BaseEvent
    {
        EventType type;
        uint8_t size;
    };

    // Payload pour les événements de mouvement de souris (4 octets)
    struct MouseEvent
    {
        int16_t deltaX;
        int16_t deltaY;
    };

    // Payload pour les événements de bouton de souris (2 octets)
    struct ButtonEvent
    {
        Action action;
        uint8_t buttonCode;
    };

    // Payload pour les événements de clavier (4 octets)
    struct KeyEvent
    {
        Action action;
        uint8_t modifiers; // Masque binaire pour Ctrl, Shift, Alt, etc.
        uint16_t keyCode;
    };

} // namespace FlowKey

#pragma pack(pop)

#endif // FLOWKEY_EVENTSTRUCTS_HPP