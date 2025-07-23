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

# Try to find OpenSSL (optional for now)
find_package(OpenSSL QUIET)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found - SSL features enabled")
    add_definitions(-DSSL_ENABLED)
else()
    message(WARNING "OpenSSL not found - SSL features disabled")
endif()

# Try to find jsoncpp (optional)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP jsoncpp QUIET)
    if(JSONCPP_FOUND)
        message(STATUS "jsoncpp found - JSON features enabled")
        add_definitions(-DJSON_ENABLED)
    else()
        message(WARNING "jsoncpp not found - using simple JSON parsing")
    endif()
else()
    message(WARNING "PkgConfig not found - jsoncpp detection skipped")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Core Source files
set(CORE_SOURCES
    src/core/TradingEngine.cpp
    src/core/ConfigManager.cpp
    src/core/OrderManager.cpp
    src/core/RenkoChart.cpp
    src/core/PatternDetector.cpp
    src/core/RiskManager.cpp
    src/core/Logger.cpp
    src/core/DatabaseManager.cpp
)

# API Source files
set(API_SOURCES
    src/api/ExchangeAPI.cpp
    src/api/BinanceAPI.cpp
    src/api/ExchangeAPIFactory.cpp
)

# UI Source files
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/RenkoChartWidget.cpp
    src/ui/MarketDataWidget.cpp
)

# Add other UI files if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/TradingDashboard.cpp")
    list(APPEND UI_SOURCES src/ui/TradingDashboard.cpp)
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
    ${API_SOURCES}
)

# Link core library dependencies
target_link_libraries(MasterMindCore
    Threads::Threads
)

# Link OpenSSL if available
if(OPENSSL_FOUND)
    target_link_libraries(MasterMindCore
        OpenSSL::SSL
        OpenSSL::Crypto
    )
endif()

# Link jsoncpp if available
if(JSONCPP_FOUND)
    target_link_libraries(MasterMindCore ${JSONCPP_LIBRARIES})
    target_include_directories(MasterMindCore PRIVATE ${JSONCPP_INCLUDE_DIRS})
    target_compile_options(MasterMindCore PRIVATE ${JSONCPP_CFLAGS_OTHER})
endif()

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
    tests/test_system.cpp
)

target_link_libraries(MasterMindTest
    MasterMindCore
    Qt6::Test
    Qt6::Core
)

# Additional test executables
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_renko.cpp")
    add_executable(MasterMindRenkoTest
        tests/test_renko.cpp
    )
    target_link_libraries(MasterMindRenkoTest
        MasterMindCore
        Qt6::Test
        Qt6::Core
    )
endif()

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

# Enable testing
enable_testing()
add_test(NAME SystemTest COMMAND MasterMindTest)

if(TARGET MasterMindRenkoTest)
    add_test(NAME RenkoTest COMMAND MasterMindRenkoTest)
endif()

# Installation
install(TARGETS MasterMindTraderGUI MasterMindTrader MasterMindTest DESTINATION bin)
install(DIRECTORY config/ DESTINATION config)
install(FILES README.md DESTINATION .)

# Create test configuration file if it doesn't exist
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/test_config.json")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/config/mastermind_config.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/config/test_config.json"
        COPYONLY
    )
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Master Mind Trading System Configuration Summary:")
message(STATUS "================================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "OpenSSL Found: ${OPENSSL_FOUND}")
message(STATUS "JsonCpp Found: ${JSONCPP_FOUND}")
message(STATUS "Core Sources: ${CORE_SOURCES}")
message(STATUS "API Sources: ${API_SOURCES}")
message(STATUS "UI Sources: ${UI_SOURCES}")
message(STATUS "")

# Warning for missing dependencies
if(NOT OPENSSL_FOUND)
    message(WARNING "OpenSSL not found. Exchange API authentication may not work.")
endif()

if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found. Please install Qt6 and set CMAKE_PREFIX_PATH correctly.")
endif() 