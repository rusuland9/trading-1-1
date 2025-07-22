cmake_minimum_required(VERSION 3.16)
project(MasterMindTradingSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6.8.3 Configuration
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Network WebSockets Sql)

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

# Core Source files (existing backend)
set(CORE_SOURCES
    src/core/TradingEngine.cpp
    src/core/RenkoChart.cpp
    src/core/PatternDetector.cpp
    src/core/OrderManager.cpp
    src/core/RiskManager.cpp
    src/core/PositionCalculator.cpp
    src/core/ConfigManager.cpp
    src/core/Logger.cpp
    src/core/DatabaseManager.cpp
)

# API Sources (existing)
set(API_SOURCES
    src/api/ExchangeAPI.cpp
    src/api/BinanceAPI.cpp
    src/api/DeribitAPI.cpp
    src/api/CoinbaseAPI.cpp
    src/api/MT4Bridge.cpp
    src/api/MT5Bridge.cpp
    src/api/WebSocketClient.cpp
    src/api/RestClient.cpp
)

# Strategy Sources (existing)
set(STRATEGY_SOURCES
    src/strategy/MasterMindStrategy.cpp
    src/strategy/SignalGenerator.cpp
    src/strategy/EntryLogic.cpp
    src/strategy/ExitLogic.cpp
)

# Utility Sources (existing)
set(UTILS_SOURCES
    src/utils/TimeUtils.cpp
    src/utils/MathUtils.cpp
    src/utils/StringUtils.cpp
    src/utils/JsonParser.cpp
)

# NEW: Qt UI Sources
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/TradingDashboard.cpp
    src/ui/RenkoChartWidget.cpp
    src/ui/OrderManagementWidget.cpp
    src/ui/RiskManagementWidget.cpp
    src/ui/PositionWidget.cpp
    src/ui/ConfigurationWidget.cpp
    src/ui/LogWidget.cpp
    src/ui/MarketDataWidget.cpp
    src/ui/PatternDetectionWidget.cpp
    src/ui/PerformanceWidget.cpp
    src/ui/ExchangeStatusWidget.cpp
)

# NEW: Qt UI Headers (will be processed by MOC)
set(UI_HEADERS
    include/ui/MainWindow.h
    include/ui/TradingDashboard.h
    include/ui/RenkoChartWidget.h
    include/ui/OrderManagementWidget.h
    include/ui/RiskManagementWidget.h
    include/ui/PositionWidget.h
    include/ui/ConfigurationWidget.h
    include/ui/LogWidget.h
    include/ui/MarketDataWidget.h
    include/ui/PatternDetectionWidget.h
    include/ui/PerformanceWidget.h
    include/ui/ExchangeStatusWidget.h
)

# NEW: Qt UI Forms
set(UI_FORMS
    src/ui/forms/MainWindow.ui
    src/ui/forms/TradingDashboard.ui
    src/ui/forms/ConfigurationWidget.ui
    src/ui/forms/OrderManagementWidget.ui
    src/ui/forms/RiskManagementWidget.ui
)

# NEW: Qt Resources
set(UI_RESOURCES
    resources/icons.qrc
    resources/styles.qrc
)

# Create main core library (existing backend)
add_library(MasterMindCore STATIC
    ${CORE_SOURCES}
    ${API_SOURCES}
    ${STRATEGY_SOURCES}
    ${UTILS_SOURCES}
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
    ${UI_HEADERS}
    ${UI_FORMS}
    ${UI_RESOURCES}
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

# Console executable (existing)
add_executable(MasterMindTrader
    src/main.cpp
)

target_link_libraries(MasterMindTrader MasterMindCore)

# Testing executable
add_executable(MasterMindTest
    tests/test_main.cpp
    tests/test_renko.cpp
    tests/test_risk_manager.cpp
    tests/test_pattern_detector.cpp
    tests/test_ui.cpp
)

target_link_libraries(MasterMindTest 
    MasterMindCore
    MasterMindUI
    Qt6::Test
)

# Qt deployment
if(WIN32)
    # Add Qt deployment for Windows
    find_program(QT_DEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(QT_DEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET MasterMindTraderGUI POST_BUILD
            COMMAND ${QT_DEPLOYQT_EXECUTABLE} $<TARGET_FILE:MasterMindTraderGUI>
            COMMENT "Qt Deploy"
        )
    endif()
endif()

# Installation
install(TARGETS MasterMindTraderGUI MasterMindTrader DESTINATION bin)
install(DIRECTORY config/ DESTINATION config)
install(DIRECTORY resources/ DESTINATION resources)
install(FILES README.md DESTINATION .)

# Copy Qt libraries for development
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET MasterMindTraderGUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt6_DIR}/../../../bin/Qt6Cored.dll"
        "${Qt6_DIR}/../../../bin/Qt6Widgetsd.dll"
        "${Qt6_DIR}/../../../bin/Qt6Chartsd.dll"
        "${Qt6_DIR}/../../../bin/Qt6Networkd.dll"
        "${Qt6_DIR}/../../../bin/Qt6WebSocketsd.dll"
        "${Qt6_DIR}/../../../bin/Qt6Sqld.dll"
        $<TARGET_FILE_DIR:MasterMindTraderGUI>
    )
endif() 