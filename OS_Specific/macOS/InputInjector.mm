// FlowKey/OS_Specific/macOS/InputInjector.mm - FINAL (Clavier & Fluidité Souris)

#include "InputInjector.hpp"
#include <ApplicationServices/ApplicationServices.h> 
#include <cmath> 
#include <iostream>

namespace FlowKey {
namespace macOS {

// --- Mappage des codes de touches Windows (VK Codes) vers macOS (CGKeyCode) ---
static CGKeyCode vk_to_cg_key(uint16_t vk_code) {
    switch (vk_code) {
        // Lettres et Chiffres (Les VK Codes sont souvent les codes ASCII majuscules correspondants)
        case 0x41: return (CGKeyCode)0x00; // A
        case 0x42: return (CGKeyCode)0x0B; // B
        case 0x43: return (CGKeyCode)0x08; // C
        case 0x44: return (CGKeyCode)0x02; // D
        case 0x45: return (CGKeyCode)0x0E; // E
        case 0x46: return (CGKeyCode)0x03; // F
        case 0x47: return (CGKeyCode)0x05; // G
        case 0x48: return (CGKeyCode)0x04; // H
        case 0x49: return (CGKeyCode)0x22; // I
        case 0x4A: return (CGKeyCode)0x26; // J
        case 0x4B: return (CGKeyCode)0x28; // K
        case 0x4C: return (CGKeyCode)0x25; // L
        case 0x4D: return (CGKeyCode)0x2E; // M
        case 0x4E: return (CGKeyCode)0x2D; // N
        case 0x4F: return (CGKeyCode)0x1F; // O
        case 0x50: return (CGKeyCode)0x23; // P
        case 0x51: return (CGKeyCode)0x0C; // Q
        case 0x52: return (CGKeyCode)0x0F; // R
        case 0x53: return (CGKeyCode)0x01; // S
        case 0x54: return (CGKeyCode)0x11; // T
        case 0x55: return (CGKeyCode)0x20; // U
        case 0x56: return (CGKeyCode)0x09; // V
        case 0x57: return (CGKeyCode)0x0D; // W
        case 0x58: return (CGKeyCode)0x07; // X
        case 0x59: return (CGKeyCode)0x10; // Y
        case 0x5A: return (CGKeyCode)0x06; // Z

        // Nombres (haut)
        case 0x30: return (CGKeyCode)0x1D; // 0
        case 0x31: return (CGKeyCode)0x12; // 1
        case 0x32: return (CGKeyCode)0x13; // 2
        case 0x33: return (CGKeyCode)0x14; // 3
        case 0x34: return (CGKeyCode)0x15; // 4
        case 0x35: return (CGKeyCode)0x17; // 5
        case 0x36: return (CGKeyCode)0x16; // 6
        case 0x37: return (CGKeyCode)0x1A; // 7
        case 0x38: return (CGKeyCode)0x1C; // 8
        case 0x39: return (CGKeyCode)0x19; // 9
        
        // Commandes
        case 0x0D: return (CGKeyCode)0x24; // Entrée (Return)
        case 0x08: return (CGKeyCode)0x33; // Retour arrière (Delete)
        case 0x20: return (CGKeyCode)0x31; // Espace

        default: return 0xFFFF; // Code inconnu
    }
}


const int MOUSE_AMPLIFICATION_FACTOR = 5; 
const int MOUSE_DEAD_ZONE_THRESHOLD = 2; // NOUVEAU: Seuil pour ignorer les petits mouvements (bruit)

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
    
    int deltaX = event.deltaX;
    int deltaY = event.deltaY;
    
    // --- FILTRAGE DE LA ZONE MORTE ---
    if (std::abs(deltaX) <= MOUSE_DEAD_ZONE_THRESHOLD && std::abs(deltaY) <= MOUSE_DEAD_ZONE_THRESHOLD) {
        // Le delta est trop petit, c'est du bruit de piégeage. On ignore.
        return; 
    }
    
    // 1. Appliquer l'amplification UNIQUEMENT si l'on est dans la zone "bruyante" mais au-dessus du seuil.
    // NOTE: On amplifie si le delta est petit mais significatif (typiquement > 2), pour vaincre l'inertie.
    if (std::abs(deltaX) > 0 && std::abs(deltaX) < 5) {
        deltaX *= MOUSE_AMPLIFICATION_FACTOR;
    }
    if (std::abs(deltaY) > 0 && std::abs(deltaY) < 5) {
        deltaY *= MOUSE_AMPLIFICATION_FACTOR;
    }
    
    // --- CORRECTION CRUCIALE : Synchroniser avec la position réelle du curseur Mac ---
    CGEventRef currentEvent = CGEventCreate(NULL);
    if (currentEvent) {
        CGPoint currentLoc = CGEventGetLocation(currentEvent);
        currentX = currentLoc.x; // Synchronise l'état interne avec la position OS
        currentY = currentLoc.y;
        CFRelease(currentEvent);
    }
    
    // 2. Appliquer le delta amplifié à la position synchronisée
    currentX += deltaX;
    currentY += deltaY;
    
    // 3. --- INJECTION ABSOLUE ---
    CGEventRef mouseMove = CGEventCreateMouseEvent(
        NULL, 
        kCGEventMouseMoved, 
        CGPointMake(round(currentX), round(currentY)), // Position absolue calculée
        kCGMouseButtonLeft 
    );

    if (mouseMove) {
        CGEventSetIntegerValueField(mouseMove, kCGMouseEventDeltaX, deltaX);
        CGEventSetIntegerValueField(mouseMove, kCGMouseEventDeltaY, deltaY);

        CGEventPost(kCGHIDEventTap, mouseMove);
        CFRelease(mouseMove);
    }
}

void InputInjector::injectButtonEvent(const FlowKey::ButtonEvent& event) {
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
            // Pour les clics, on utilise la position ABSOLUE SYNCHRONISÉE
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
    CGKeyCode mac_code = vk_to_cg_key(event.keyCode);

    if (mac_code != 0xFFFF) {
        bool is_down = (event.action == FlowKey::Action::PRESS);

        // Créer l'événement clavier
        CGEventRef keyEvent = CGEventCreateKeyboardEvent(
            NULL,
            mac_code, 
            is_down
        );

        if (keyEvent) {
            // NOTE: La gestion des modificateurs (SHIFT, CTRL, etc.) est ignorée pour l'instant
            
            CGEventPost(kCGHIDEventTap, keyEvent);
            CFRelease(keyEvent);
        }
    }
}

} // namespace macOS
} // namespace FlowKeywqe