cmake_minimum_required(VERSION 3.16)
project(MasterMindTradingSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6.4.2 Configuration (installed via Chocolatey)
set(CMAKE_PREFIX_PATH "C:/Qt/6.4.2/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Network Sql Test)

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Compiler-specific options for performance
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /DDEBUG")
    # Required for Qt6 with MSVC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /permissive-")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Try to find OpenSSL (optional for now)
find_package(OpenSSL QUIET)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found - SSL features enabled")
    add_definitions(-DSSL_ENABLED)
else()
    message(WARNING "OpenSSL not found - SSL features disabled")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core Source files
set(CORE_SOURCES
    src/core/TradingEngine.cpp
    src/core/ConfigManager.cpp
    src/core/OrderManager.cpp
    src/core/RenkoChart.cpp
    src/core/PatternDetector.cpp
    src/core/RiskManager.cpp
    src/core/DatabaseManager.cpp
    src/core/Logger.cpp
)

# UI Source files
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/MainWindow_Part2.cpp
    src/ui/RenkoChartWidget.cpp
    src/ui/MarketDataWidget.cpp
    src/ui/OrderManagementWidget.cpp
    src/ui/RiskManagementWidget.cpp
    src/ui/PositionWidget.cpp
)

# Create core library
add_library(MasterMindCore ${CORE_SOURCES})
target_link_libraries(MasterMindCore Threads::Threads)

if(OPENSSL_FOUND)
    target_link_libraries(MasterMindCore OpenSSL::SSL OpenSSL::Crypto)
endif()

# Console executable
add_executable(MasterMindTrader
    src/main.cpp
)

# GUI executable
add_executable(MasterMindTraderGUI
    src/main_gui.cpp
    ${UI_SOURCES}
)

# Demo simulation executable
add_executable(DemoSimulation
    demo_simulation.cpp
)

# Link libraries
target_link_libraries(MasterMindTrader MasterMindCore)
target_link_libraries(DemoSimulation MasterMindCore)

# Link Qt libraries to GUI executable
target_link_libraries(MasterMindTraderGUI 
    MasterMindCore
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    Qt6::Sql
)

# Set subsystem for GUI application
if(WIN32)
    set_target_properties(MasterMindTraderGUI PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Installation
install(TARGETS MasterMindTrader MasterMindTraderGUI DemoSimulation DESTINATION bin)
install(DIRECTORY config/ DESTINATION config)
install(FILES README.md DESTINATION .) 