// FlowKey/OS_Specific/macOS/InputInjector.mm - Implémentation de l'injection macOS

#include "InputInjector.hpp"
#include <ApplicationServices/ApplicationServices.h> 
#include <cmath> // Pour la fonction round

namespace FlowKey {
namespace macOS {

InputInjector::InputInjector() {
    // Initialise la position actuelle du curseur lors de l'instanciation
    CGEventRef currentEvent = CGEventCreate(NULL);
    if (currentEvent) {
        CGPoint currentLoc = CGEventGetLocation(currentEvent);
        currentX = currentLoc.x;
        currentY = currentLoc.y;
        CFRelease(currentEvent);
    } else {
        currentX = 0.0;
        currentY = 0.0;
    }
}

void InputInjector::injectMouseMove(const FlowKey::MouseEvent& event) {
    // 1. Récupérer la position actuelle (pour prendre en compte les mouvements locaux)
    CGEventRef currentEvent = CGEventCreate(NULL);
    if (!currentEvent) return;

    CGPoint currentLoc = CGEventGetLocation(currentEvent);
    CFRelease(currentEvent);

    currentX = currentLoc.x;
    currentY = currentLoc.y;

    // 2. Appliquer le déplacement delta reçu du Serveur
    currentX += event.deltaX;
    currentY += event.deltaY;
    
    // 3. Clamper la position aux limites de l'écran (non implémenté ici, mais crucial pour la Phase 3)

    // 4. Créer l'événement de mouvement (absolu)
    CGEventRef mouseMove = CGEventCreateMouseEvent(
        NULL, 
        kCGEventMouseMoved, 
        CGPointMake(round(currentX), round(currentY)), 
        kCGMouseButtonLeft // Bouton non utilisé pour le mouvement
    );

    if (mouseMove) {
        // 5. Injecter l'événement
        CGEventPost(kCGHIDEventTap, mouseMove);
        CFRelease(mouseMove);
    }
}

void InputInjector::injectButtonEvent(const FlowKey::ButtonEvent& event) {
    CGEventType type = kCGEventNull;
    CGMouseButton button = kCGMouseButtonLeft;
    
    // Convertir le code du bouton
    if (event.buttonCode == 1) { // Clic gauche
        button = kCGMouseButtonLeft;
        type = (event.action == FlowKey::Action::PRESS) ? kCGEventLeftMouseDown : kCGEventLeftMouseUp;
    } else if (event.buttonCode == 2) { // Clic droit
        button = kCGMouseButtonRight;
        type = (event.action == FlowKey::Action::PRESS) ? kCGEventRightMouseDown : kCGEventRightMouseUp;
    } else if (event.buttonCode == 3) { // Clic milieu
        button = kCGMouseButtonCenter;
        type = (event.action == FlowKey::Action::PRESS) ? kCGEventOtherMouseDown : kCGEventOtherMouseUp;
    } 
    
    if (type != kCGEventNull) {
        // Créer et injecter l'événement de bouton
        CGEventRef buttonEvent = CGEventCreateMouseEvent(
            NULL,
            type,
            CGPointMake(round(currentX), round(currentY)), // Utiliser la dernière position connue
            button
        );
        
        if (buttonEvent) {
            CGEventPost(kCGHIDEventTap, buttonEvent);
            CFRelease(buttonEvent);
        }
    }
    
    if (event.action == FlowKey::Action::SCROLL_UP || event.action == FlowKey::Action::SCROLL_DOWN) {
        // Événement de molette (Défilement)
        int direction = (event.action == FlowKey::Action::SCROLL_UP) ? 1 : -1;
        
        CGEventRef scrollEvent = CGEventCreateScrollWheelEvent(
            NULL, 
            kCGScrollEventUnitLine, 
            1, 
            direction * 3 // Vitesse de défilement
        );
        
        if (scrollEvent) {
            CGEventPost(kCGHIDEventTap, scrollEvent);
            CFRelease(scrollEvent);
        }
    }
}

void InputInjector::injectKeyEvent(const FlowKey::KeyEvent& event) {
    // NOTE: L'injection de clavier est laissée vide pour le moment
    // car elle nécessite une table de mapping complexe des codes de touches.
}

} // namespace macOS
} // namespace FlowKey