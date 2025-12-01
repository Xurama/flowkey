# FlowKey/CMakeLists.txt (Avec compilation conditionnelle)

cmake_minimum_required(VERSION 3.10)
project(FlowKey VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CORE_SOURCES
    Core/NetworkManager.cpp
)

# --- Configuration BOOST Globale ---
# On ne demande pas les composants binaires tout de suite pour éviter l'échec immédiat.
# On demande uniquement le chemin des headers.
find_package(Boost 1.70 REQUIRED) 

# Le chemin d'inclusion des headers est ajouté
include_directories(/opt/homebrew/include)


# --- SECTION SERVEUR (Windows UNIQUEMENT) ---

# Le bloc WIN32 couvre les systèmes Windows, y compris MinGW
if(WIN32)
    # Pour Windows, nous devons chercher les composants binaires nécessaires.
    find_package(Boost 1.70 COMPONENTS system thread REQUIRED)

    if(Boost_FOUND)
        message(STATUS "Configuration Serveur (Windows) prête.")
        add_executable(FlowKey_Server Server/main.cpp ${CORE_SOURCES}) 
        target_link_libraries(FlowKey_Server PRIVATE Boost::system Boost::thread)
    else()
        message(FATAL_ERROR "Boost n'est pas configuré correctement sur ce système Windows.")
    endif()
endif()


# --- SECTION CLIENT (macOS UNIQUEMENT) ---

# Le bloc APPLE couvre les systèmes macOS
if(APPLE)
    message(STATUS "Configuration Client (macOS) prête.")

    # Pour le Client Mac, nous utilisons la solution de contournement (Header-Only Asio)
    add_executable(FlowKey_Client Client/main.cpp ${CORE_SOURCES})
    
    # Nous lions manuellement la librairie de threads native d'Apple (pthread)
    target_link_libraries(FlowKey_Client PRIVATE pthread) 

    # Assurez-vous d'avoir installé les Command Line Tools et Boost sur Mac.
    
endif()


# Définir le répertoire de sortie pour les exécutables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)