// FlowKey/OS_Specific/macOS/InputInjector.mm - Implémentation de l'injection macOS

#include "InputInjector.hpp"
#include <ApplicationServices/ApplicationServices.h> 
#include <cmath> 

namespace FlowKey {
namespace macOS {

InputInjector::InputInjector() {
    // L'état initial n'est plus critique pour le mouvement, mais est gardé pour les clics
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
    // --- CORRECTION CRUCIALE : Utiliser le mouvement relatif ---
    
    // Le mouvement relatif est injecté en utilisant l'API IOEventSource, 
    // qui est plus fiable que CGEventCreateMouseEvent pour les deltas.
    
    // 1. Créer un événement de déplacement relatif (kCGEventMouseMoved)
    CGEventRef mouseMove = CGEventCreateMouseEvent(
        NULL, 
        kCGEventMouseMoved, 
        CGPointMake(0, 0), // La position absolue est ignorée en mode relatif
        kCGMouseButtonLeft // Bouton non utilisé pour le mouvement
    );

    if (mouseMove) {
        // 2. Définir le déplacement DELTA (le mouvement reçu)
        // Les champs kCGEventMouseDeltaX/Y indiquent au système que l'événement est un delta.
        CGEventSetIntegerValueField(mouseMove, kCGMouseEventDeltaX, event.deltaX);
        CGEventSetIntegerValueField(mouseMove, kCGMouseEventDeltaY, event.deltaY);
        
        // 3. Injecter l'événement (kCGHIDEventTap est le plus approprié)
        CGEventPost(kCGHIDEventTap, mouseMove);
        CFRelease(mouseMove);
    }

    // Mettre à jour la position absolue pour les événements de clic (qui sont toujours absolus)
    currentX += event.deltaX;
    currentY += event.deltaY;
    
    // NOTE: Si le curseur du Mac sort, il faudra renvoyer un signal au Serveur Windows (Phase 4).
}

// Les fonctions injectButtonEvent et injectKeyEvent restent inchangées 
// (elles utilisent currentX/Y pour le positionnement ABSOLU du clic, ce qui est correct).

void InputInjector::injectButtonEvent(const FlowKey::ButtonEvent& event) {
    // ... code inchangé ...
    // Le code du clic est correct, il utilise la dernière position absolue connue (currentX, currentY).
    CGEventType type = kCGEventNull;
    CGMouseButton button = kCGMouseButtonLeft;
    
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
        CGEventRef buttonEvent = CGEventCreateMouseEvent(
            NULL,
            type,
            // Utilise la dernière position mise à jour par le delta relatif
            CGPointMake(round(currentX), round(currentY)), 
            button
        );
        
        if (buttonEvent) {
            CGEventPost(kCGHIDEventTap, buttonEvent);
            CFRelease(buttonEvent);
        }
    }
    
    if (event.action == FlowKey::Action::SCROLL_UP || event.action == FlowKey::Action::SCROLL_DOWN) {
        // ... code inchangé ...
        int direction = (event.action == FlowKey::Action::SCROLL_UP) ? 1 : -1;
        
        CGEventRef scrollEvent = CGEventCreateScrollWheelEvent(
            NULL, 
            kCGScrollEventUnitLine, 
            1, 
            direction * 3 
        );
        
        if (scrollEvent) {
            CGEventPost(kCGHIDEventTap, scrollEvent);
            CFRelease(scrollEvent);
        }
    }
}

void InputInjector::injectKeyEvent(const FlowKey::KeyEvent& event) {
    // ... reste inchangé
}

} // namespace macOS
} // namespace FlowKey