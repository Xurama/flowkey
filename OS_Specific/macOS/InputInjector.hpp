// FlowKey/OS_Specific/macOS/InputInjector.hpp

#ifndef FLOWKEY_MAC_INPUTINJECTOR_HPP
#define FLOWKEY_MAC_INPUTINJECTOR_HPP

#include "../../Core/EventStructs.hpp"

namespace FlowKey {
namespace macOS {

/**
 * @class InputInjector
 * @brief Gère l'injection d'événements de souris et de clavier dans macOS.
 */
class InputInjector {
public:
    InputInjector();
    ~InputInjector() = default;

    /**
     * @brief Injecte un événement de mouvement de souris.
     * @param event L'événement MouseEvent (delta X/Y) à injecter.
     */
    void injectMouseMove(const FlowKey::MouseEvent& event);

    /**
     * @brief Injecte un événement de bouton de souris ou de molette.
     */
    void injectButtonEvent(const FlowKey::ButtonEvent& event);

    /**
     * @brief Injecte un événement de clavier (Bientôt implémenté).
     */
    void injectKeyEvent(const FlowKey::KeyEvent& event);

private:
    // État du curseur pour l'injection relative/absolue
    double currentX;
    double currentY;
};

} // namespace macOS
} // namespace FlowKey

#endif // FLOWKEY_MAC_INPUTINJECTOR_HPP