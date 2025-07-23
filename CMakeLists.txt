cmake_minimum_required(VERSION 3.16)
project(MasterMindTradingSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6.8.3 Configuration
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Network WebSockets Sql Test)

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
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Core Source files (only existing files)
set(CORE_SOURCES
    src/core/TradingEngine.cpp
    src/core/ConfigManager.cpp
    src/core/OrderManager.cpp
)

# Add other core files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/RenkoChart.cpp")
    list(APPEND CORE_SOURCES src/core/RenkoChart.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/PatternDetector.cpp")
    list(APPEND CORE_SOURCES src/core/PatternDetector.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/RiskManager.cpp")
    list(APPEND CORE_SOURCES src/core/RiskManager.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Logger.cpp")
    list(APPEND CORE_SOURCES src/core/Logger.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/DatabaseManager.cpp")
    list(APPEND CORE_SOURCES src/core/DatabaseManager.cpp)
endif()

# UI Sources (only existing files)
set(UI_SOURCES
    src/ui/MainWindow.cpp
)

# Add other UI files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/TradingDashboard.cpp")
    list(APPEND UI_SOURCES src/ui/TradingDashboard.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/RenkoChartWidget.cpp")
    list(APPEND UI_SOURCES src/ui/RenkoChartWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/OrderManagementWidget.cpp")
    list(APPEND UI_SOURCES src/ui/OrderManagementWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/RiskManagementWidget.cpp")
    list(APPEND UI_SOURCES src/ui/RiskManagementWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PositionWidget.cpp")
    list(APPEND UI_SOURCES src/ui/PositionWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ConfigurationWidget.cpp")
    list(APPEND UI_SOURCES src/ui/ConfigurationWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/LogWidget.cpp")
    list(APPEND UI_SOURCES src/ui/LogWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MarketDataWidget.cpp")
    list(APPEND UI_SOURCES src/ui/MarketDataWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PatternDetectionWidget.cpp")
    list(APPEND UI_SOURCES src/ui/PatternDetectionWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PerformanceWidget.cpp")
    list(APPEND UI_SOURCES src/ui/PerformanceWidget.cpp)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ExchangeStatusWidget.cpp")
    list(APPEND UI_SOURCES src/ui/ExchangeStatusWidget.cpp)
endif()

# Create main core library
add_library(MasterMindCore STATIC
    ${CORE_SOURCES}
)

# Link core library dependencies
target_link_libraries(MasterMindCore
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Create Qt UI library
add_library(MasterMindUI STATIC
    ${UI_SOURCES}
)

# Link Qt UI library dependencies
target_link_libraries(MasterMindUI
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
    MasterMindCore
)

# Main GUI executable
add_executable(MasterMindTraderGUI WIN32
    src/main_gui.cpp
)

target_link_libraries(MasterMindTraderGUI
    MasterMindUI
    MasterMindCore
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
)

# Console executable
add_executable(MasterMindTrader
    src/main.cpp
)

target_link_libraries(MasterMindTrader MasterMindCore)

# Test executable
add_executable(MasterMindTest
    tests/test_renko.cpp
)

target_link_libraries(MasterMindTest
    MasterMindCore
    Qt6::Test
    Qt6::Core
)

# Qt deployment for Windows
if(WIN32)
    find_program(QT_DEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(QT_DEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET MasterMindTraderGUI POST_BUILD
            COMMAND ${QT_DEPLOYQT_EXECUTABLE} $<TARGET_FILE:MasterMindTraderGUI>
            COMMENT "Qt Deploy"
        )
    endif()
endif()

# Installation
install(TARGETS MasterMindTraderGUI MasterMindTrader MasterMindTest DESTINATION bin)
install(DIRECTORY config/ DESTINATION config)
install(FILES README.md DESTINATION .) 