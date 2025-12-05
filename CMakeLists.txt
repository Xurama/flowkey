# FlowKey/CMakeLists.txt (Définit les règles de compilation pour les deux OS)

cmake_minimum_required(VERSION 3.10)
project(FlowKey VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CORE_SOURCES
    Core/NetworkManager.cpp
)

# Chemin d'inclusion général (pour le Client Mac où Boost est Header-Only)
include_directories(/opt/homebrew/include)


# --- SECTION SERVEUR (Windows UNIQUEMENT) ---
if(WIN32)
    # Configure les chemins spécifiques à MinGW (résout les problèmes de Boost non trouvé)
    set(BOOST_ROOT "$ENV{MSYSTEM_PREFIX}")
    set(CMAKE_PREFIX_PATH "$ENV{MSYSTEM_PREFIX}")

    find_package(Boost 1.70 COMPONENTS system thread REQUIRED)

    if(Boost_FOUND)
        message(STATUS "Configuration Serveur (Windows) prête.")
        add_executable(FlowKey_Server Server/main.cpp ${CORE_SOURCES}) 
        
        # Liaison des librairies Boost + la librairie réseau Windows (ws2_32)
        target_link_libraries(FlowKey_Server PRIVATE 
            Boost::system 
            Boost::thread
            ws2_32 
        )
    else()
        message(FATAL_ERROR "Boost (librairies binaires) n'a pas été trouvé pour le Serveur Windows.")
    endif()
endif()


# --- SECTION CLIENT (macOS UNIQUEMENT) ---
if(APPLE)
    message(STATUS "Configuration Client (macOS) prête.")
    
    # Sources du Client Mac (inclut le nouveau fichier Objective-C++)
    set(MAC_CLIENT_SOURCES 
        Client/main.cpp
        OS_Specific/macOS/InputInjector.mm
    )

    add_executable(FlowKey_Client ${MAC_CLIENT_SOURCES} ${CORE_SOURCES})
    
    # Lier la librairie de threads et les frameworks macOS nécessaires pour CoreGraphics
    target_link_libraries(FlowKey_Client PRIVATE 
        pthread 
        "-framework IOKit" 
        "-framework CoreGraphics"
    )
    
    # Indiquer à CMake que InputInjector.mm doit être compilé comme Objective-C++
    set_source_files_properties(OS_Specific/macOS/InputInjector.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()


# Définir le répertoire de sortie pour les exécutables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)