# FlowKey/CMakeLists.txt (Ajout du Framework CoreFoundation)

cmake_minimum_required(VERSION 3.10)
project(FlowKey VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CORE_SOURCES
    Core/NetworkManager.cpp
)

include_directories(/opt/homebrew/include)


# --- SECTION SERVEUR (Windows UNIQUEMENT) ---
if(WIN32)
    # Configure les chemins spécifiques à MinGW
    set(BOOST_ROOT "$ENV{MSYSTEM_PREFIX}")
    set(CMAKE_PREFIX_PATH "$ENV{MSYSTEM_PREFIX}")

    find_package(Boost 1.70 COMPONENTS system thread REQUIRED)

    if(Boost_FOUND)
        message(STATUS "Configuration Serveur (Windows) prête.")
        
        set(WINDOWS_SERVER_SOURCES 
            Server/main.cpp
            OS_Specific/Windows/InputInterceptor.cpp
        )

        add_executable(FlowKey_Server ${WINDOWS_SERVER_SOURCES} ${CORE_SOURCES}) 
        
        target_link_libraries(FlowKey_Server PRIVATE 
            Boost::system 
            Boost::thread
            ws2_32 
            user32 
        )
    else()
        message(FATAL_ERROR "Boost (librairies binaires) n'a pas été trouvé pour le Serveur Windows.")
    endif()
endif()


# --- SECTION CLIENT (macOS UNIQUEMENT) ---
if(APPLE)
    message(STATUS "Configuration Client (macOS) prête.")
    
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
        "-framework CoreFoundation" # NOUVEAU: Obligatoire pour CFRelease
    )
    
    set_source_files_properties(OS_Specific/macOS/InputInjector.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()


# Définir le répertoire de sortie pour les exécutables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)