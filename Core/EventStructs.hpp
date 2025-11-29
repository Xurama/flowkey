#include <cstdint>

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

    struct BaseEvent
    {
        EventType type;
        uint8_t size;
    };

    struct MouseEvent
    {
        int16_t deltaX;
        int16_t deltaY;
    };

    struct ButtonEvent
    {
        Action action;
        uint8_t buttonCode;
    };

    struct KeyEvent
    {
        Action action;
        uint8_t modifiers;
        uint16_t keyCode;
    };

}

#pragma pack(pop)