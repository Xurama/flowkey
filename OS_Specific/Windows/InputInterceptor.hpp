// FlowKey/OS_Specific/Windows/InputInterceptor.hpp

#ifndef FLOWKEY_WINDOWS_INPUTINTERCEPTOR_HPP
#define FLOWKEY_WINDOWS_INPUTINTERCEPTOR_HPP

#include <windows.h>
#include <functional>
#include "../../Core/EventStructs.hpp"

namespace FlowKey {
namespace Windows {

using EventCallback = std::function<void(EventType, const void*, uint8_t)>;

/**
 * @class InputInterceptor
 * @brief Gère la capture des événements de souris et de clavier via Windows Hooks.
 */
class InputInterceptor {
public:
    InputInterceptor(EventCallback callback);
    ~InputInterceptor();

    void startEventLoop();
    void stopEventLoop();

private:
    HHOOK mouseHook;
    HHOOK keyboardHook;
    EventCallback eventCallback;

    // --- CORRECTION : Les fonctions de basculement sont maintenant des méthodes statiques ---
    static void switchToServer();
    static void switchToClient();
    
    // Fonction pour l'envoi du mouvement (reste statique)
    static void checkEdgeAndSend(int deltaX, int deltaY, POINT currentPoint);
    
    // Fonctions statiques de rappel pour les hooks Windows
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    // Pointeur statique vers l'instance de la classe
    static InputInterceptor* s_instance;
};

} // namespace Windows
} // namespace FlowKey

#endif // FLOWKEY_WINDOWS_INPUTINTERCEPTOR_HPP