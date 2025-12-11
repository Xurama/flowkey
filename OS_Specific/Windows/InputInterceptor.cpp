// FlowKey/OS_Specific/Windows/InputInterceptor.cpp - Logique d'Interception d'Entrée et de Basculement

#include "InputInterceptor.hpp"
#include <iostream>

namespace FlowKey {
namespace Windows {

// Initialisation du pointeur statique à nullptr
InputInterceptor* InputInterceptor::s_instance = nullptr;

// Variables pour le suivi de la position de la souris et l'état de contrôle
static int s_virtualScreenWidth = 0; 
static bool s_isControllingClient = false;
static POINT s_lastPoint = {0, 0};
static const int EDGE_BUFFER_WIDTH = 10; 

InputInterceptor::InputInterceptor(EventCallback callback) : eventCallback(std::move(callback)) {
    if (s_instance) {
        throw std::runtime_error("InputInterceptor déjà instancié.");
    }
    s_instance = this;
    
    s_virtualScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    
    if (!GetCursorPos(&s_lastPoint)) {
        s_lastPoint = {0, 0};
    }

    std::cout << "Largeur virtuelle totale des écrans: " << s_virtualScreenWidth << " pixels." << std::endl;
    std::cout << "--- UTILISER SHIFT + CTRL + F12 POUR BASCULER ---" << std::endl;

    // Installer les hooks
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    if (!mouseHook || !keyboardHook) {
        std::cerr << "Erreur: Échec de l'installation des hooks d'entrée." << std::endl;
        stopEventLoop();
        throw std::runtime_error("Échec de l'installation des hooks.");
    }
}

InputInterceptor::~InputInterceptor() {
    stopEventLoop();
    s_instance = nullptr;
}

void InputInterceptor::stopEventLoop() {
    if (mouseHook) {
        UnhookWindowsHookEx(mouseHook);
        mouseHook = NULL;
    }
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }
}

void InputInterceptor::startEventLoop() {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void InputInterceptor::switchToServer() {
    if (s_isControllingClient) {
        s_isControllingClient = false;
        std::cout << "<<< BASCULEMENT VERS SERVEUR (Windows) >>>" << std::endl;
        
        SetCursorPos(s_virtualScreenWidth - EDGE_BUFFER_WIDTH - 1, s_lastPoint.y); 
    }
}

void InputInterceptor::switchToClient() {
    if (!s_isControllingClient) {
        if (!s_instance || !s_instance->eventCallback) return; 

        s_isControllingClient = true;
        std::cout << ">>> BASCULEMENT VERS CLIENT (Mac) <<<" << std::endl;
        
        SetCursorPos(s_virtualScreenWidth - EDGE_BUFFER_WIDTH - 1, s_lastPoint.y); 
        
        FlowKey::MouseEvent mouseEvent;
        mouseEvent.deltaX = 20; 
        mouseEvent.deltaY = 0;
        s_instance->eventCallback(FlowKey::EventType::MOUSE_MOVE, &mouseEvent, sizeof(FlowKey::MouseEvent));
    }
}

// Fonction de basculement de bordure (désactivée pour l'instant)
void InputInterceptor::checkEdgeAndSend(int deltaX, int deltaY, POINT currentPoint) {
    if (!s_instance || !s_instance->eventCallback) return;

    // --- ENVOI DE L'ÉVÉNEMENT (si le contrôle est sur le Client) ---
    if (s_isControllingClient) {
        // --- FILTRAGE CRITIQUE : NE PAS ENVOYER LES DELTAS 0/0 ---
        if (deltaX == 0 && deltaY == 0) {
            return;
        }

        FlowKey::MouseEvent mouseEvent;
        mouseEvent.deltaX = (int16_t)deltaX;
        mouseEvent.deltaY = (int16_t)deltaY;
        s_instance->eventCallback(FlowKey::EventType::MOUSE_MOVE, &mouseEvent, sizeof(FlowKey::MouseEvent));
    }
}


LRESULT CALLBACK InputInterceptor::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && s_instance) {
        MSLLHOOKSTRUCT* hookStruct = (MSLLHOOKSTRUCT*)lParam;
        POINT currentPoint = hookStruct->pt;
        
        int deltaX = currentPoint.x - s_lastPoint.x;
        int deltaY = currentPoint.y - s_lastPoint.y;
        
        // 1. Gérer le Mouvement
        if (wParam == WM_MOUSEMOVE) {
            
            InputInterceptor::checkEdgeAndSend(deltaX, deltaY, currentPoint);
            s_lastPoint = currentPoint;
            
            // Si nous sommes en mode client, nous bloquons l'événement local
            if (s_isControllingClient) {
                 return 1; // BLOQUER LE MOUVEMENT LOCAL
            }
        }
        
        // 2. Traiter les Boutons (si le contrôle est sur le client)
        if (s_isControllingClient) {
            switch (wParam) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP: {
                    FlowKey::ButtonEvent buttonEvent;
                    buttonEvent.action = (wParam == WM_LBUTTONDOWN) ? FlowKey::Action::PRESS : FlowKey::Action::RELEASE;
                    buttonEvent.buttonCode = 1; 
                    s_instance->eventCallback(FlowKey::EventType::MOUSE_BUTTON, &buttonEvent, sizeof(FlowKey::ButtonEvent));
                    return 1; // BLOQUER L'ÉVÉNEMENT LOCAL
                }
                // ... autres boutons
            }
        }
    }
    // Sinon, passer l'événement au hook suivant
    return CallNextHookEx(s_instance ? s_instance->mouseHook : NULL, nCode, wParam, lParam);
}


LRESULT CALLBACK InputInterceptor::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && s_instance) {
        KBDLLHOOKSTRUCT* hookStruct = (KBDLLHOOKSTRUCT*)lParam;
        FlowKey::KeyEvent keyEvent;

        // --- DÉTECTION DU RACCOURCI DE BASCULEMENT (SHIFT + CTRL + F12) ---
        if ((hookStruct->vkCode == VK_F12) && (wParam == WM_KEYDOWN)) {
            bool isCtrlPressed = GetAsyncKeyState(VK_CONTROL) & 0x8000;
            bool isShiftPressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;
            
            if (isCtrlPressed && isShiftPressed) {
                if (s_isControllingClient) {
                    InputInterceptor::switchToServer(); // Appel corrigé
                } else {
                    InputInterceptor::switchToClient(); // Appel corrigé
                }
                return 1; // Bloquer la touche F12
            }
        }
        
        // Si nous sommes en mode client, intercepter et envoyer TOUTES les touches
        if (s_isControllingClient) {
            
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                keyEvent.action = FlowKey::Action::PRESS;
            } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                keyEvent.action = FlowKey::Action::RELEASE;
            } else {
                return CallNextHookEx(s_instance->keyboardHook, nCode, wParam, lParam);
            }

            keyEvent.keyCode = (uint16_t)hookStruct->vkCode; 
            keyEvent.modifiers = 0; 

            s_instance->eventCallback(FlowKey::EventType::KEYBOARD, &keyEvent, sizeof(FlowKey::KeyEvent));
            
            return 1; // BLOQUER L'ÉVÉNEMENT LOCAL
        }
    }
    return CallNextHookEx(s_instance ? s_instance->keyboardHook : NULL, nCode, wParam, lParam);
}

} // namespace Windows
} // namespace FlowKey