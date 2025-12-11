// FlowKey/OS_Specific/macOS/InputInjector.mm - Optimisation pour la Fluidité

#include "InputInjector.hpp"
#include <ApplicationServices/ApplicationServices.h> 
#include <cmath> 
#include <iostream>

namespace FlowKey {
namespace macOS {

// Suppression du facteur MOUSE_AMPLIFICATION_FACTOR

InputInjector::InputInjector() {
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
    
    // Tenter de débloquer le curseur
    CGWarpMouseCursorPosition(CGPointMake(currentX, currentY));
}

void InputInjector::injectMouseMove(const FlowKey::MouseEvent& event) {
    
    // 1. Utiliser les deltas bruts
    int deltaX = event.deltaX;
    int deltaY = event.deltaY;

    // 2. Mettre à jour la position absolue interne (POUR LES CLICS)
    currentX += deltaX;
    currentY += deltaY;

    // 3. --- INJECTION ABSOLUE (pour la fluidité) ---
    // Cette méthode utilise le lissage natif du système.
    CGEventRef mouseMove = CGEventCreateMouseEvent(
        NULL, 
        kCGEventMouseMoved, 
        CGPointMake(round(currentX), round(currentY)), // Utilise la position absolue calculée
        kCGMouseButtonLeft 
    );

    if (mouseMove) {
        // Optionnel : Inclure les deltas, bien que le mouvement soit basé sur la position
        CGEventSetIntegerValueField(mouseMove, kCGMouseEventDeltaX, deltaX);
        CGEventSetIntegerValueField(mouseMove, kCGMouseEventDeltaY, deltaY);

        CGEventPost(kCGHIDEventTap, mouseMove);
        CFRelease(mouseMove);
    }
}

void InputInjector::injectButtonEvent(const FlowKey::ButtonEvent& event) {
    // ... (Reste inchangé)
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
            CGPointMake(round(currentX), round(currentY)), 
            button
        );
        
        if (buttonEvent) {
            CGEventPost(kCGHIDEventTap, buttonEvent);
            CFRelease(buttonEvent);
        }
    }
    
    if (event.action == FlowKey::Action::SCROLL_UP || event.action == FlowKey::Action::SCROLL_DOWN) {
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
    // Reste inchangé
}

} // namespace macOS
} // namespace FlowKey