# FlowKey/CMakeLists.txt (Ajout de ws2_32 pour Windows)

cmake_minimum_required(VERSION 3.10)
project(FlowKey VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CORE_SOURCES
    Core/NetworkManager.cpp
)

# Chemin d'inclusion général (pour le Client Mac où c'est Header-Only)
include_directories(/opt/homebrew/include)


# --- SECTION SERVEUR (Windows UNIQUEMENT) ---
if(WIN32)
    # AJOUT CRUCIAL : Définir les chemins de recherche spécifiques à MinGW
    set(BOOST_ROOT "$ENV{MSYSTEM_PREFIX}")
    set(CMAKE_PREFIX_PATH "$ENV{MSYSTEM_PREFIX}")

    # Rechercher les composants binaires nécessaires pour le Serveur Windows
    find_package(Boost 1.70 COMPONENTS system thread REQUIRED)

    if(Boost_FOUND)
        message(STATUS "Configuration Serveur (Windows) prête.")
        add_executable(FlowKey_Server Server/main.cpp ${CORE_SOURCES}) 
        
        # --- LIAISON CORRIGÉE : AJOUT DE ws2_32 ---
        target_link_libraries(FlowKey_Server PRIVATE 
            Boost::system 
            Boost::thread
            ws2_32 # Bibliothèque Windows Sockets (WSA) pour les fonctions réseau
        )
    else()
        message(FATAL_ERROR "Boost (librairies binaires) n'a pas été trouvé pour le Serveur Windows. Vérifiez que le paquet 'mingw-w64-x86_64-boost' est installé.")
    endif()
endif()


# --- SECTION CLIENT (macOS UNIQUEMENT) ---
if(APPLE)
    message(STATUS "Configuration Client (macOS) prête.")
    add_executable(FlowKey_Client Client/main.cpp ${CORE_SOURCES})
    
    # Sur macOS, Asio utilise la librairie pthread et la librairie système
    target_link_libraries(FlowKey_Client PRIVATE pthread) 
endif()


# Définir le répertoire de sortie pour les exécutables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)