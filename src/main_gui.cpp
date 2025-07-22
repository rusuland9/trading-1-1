#include "ui/MainWindow.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSplashScreen>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QLoggingCategory>
#include <QtGui/QPixmap>
#include <QtGui/QFont>
#include <QtCore/QThread>
#include <iostream>

Q_LOGGING_CATEGORY(gui, "mastermind.gui")

/**
 * @brief Initialize application directories and resources
 */
bool initializeApplicationEnvironment()
{
    // Create necessary directories
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir;
    
    if (!dir.mkpath(appDataDir + "/logs")) {
        qCritical(gui) << "Failed to create logs directory:" << appDataDir + "/logs";
        return false;
    }
    
    if (!dir.mkpath(appDataDir + "/config")) {
        qCritical(gui) << "Failed to create config directory:" << appDataDir + "/config";
        return false;
    }
    
    if (!dir.mkpath(appDataDir + "/database")) {
        qCritical(gui) << "Failed to create database directory:" << appDataDir + "/database";
        return false;
    }
    
    if (!dir.mkpath(appDataDir + "/reports")) {
        qCritical(gui) << "Failed to create reports directory:" << appDataDir + "/reports";
        return false;
    }
    
    qInfo(gui) << "Application directories initialized at:" << appDataDir;
    return true;
}

/**
 * @brief Set up application-wide styling and fonts
 */
void setupApplicationStyle(QApplication &app)
{
    // Set application properties
    app.setApplicationName("Master Mind Trading System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Master Mind Trading");
    app.setOrganizationDomain("mastermindtrading.com");

    // Set high DPI support
    app.setAttribute(Qt::AA_EnableHighDpiScaling, true);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    // Set default font
    QFont defaultFont = app.font();
    defaultFont.setFamily("Segoe UI");
    defaultFont.setPointSize(9);
    app.setFont(defaultFont);

    // Apply initial dark theme stylesheet
    QString darkStyleSheet = R"(
        /* Main Application Style */
        QMainWindow {
            background-color: #1e1e2e;
            color: #ffffff;
        }
        
        /* Dock Widgets */
        QDockWidget {
            background-color: #2d2d3a;
            color: #ffffff;
            titlebar-close-icon: url(:/icons/close.png);
            titlebar-normal-icon: url(:/icons/undock.png);
        }
        
        QDockWidget::title {
            background-color: #3d3d4a;
            padding: 5px;
            border: 1px solid #4d4d5a;
        }
        
        /* Tool Bar */
        QToolBar {
            background-color: #2d2d3a;
            border: 1px solid #4d4d5a;
            spacing: 3px;
            padding: 2px;
        }
        
        /* Menu Bar */
        QMenuBar {
            background-color: #2d2d3a;
            color: #ffffff;
            border: 1px solid #4d4d5a;
        }
        
        QMenuBar::item {
            background: transparent;
            padding: 4px 8px;
        }
        
        QMenuBar::item:selected {
            background-color: #00d4ff;
            color: #000000;
        }
        
        /* Menus */
        QMenu {
            background-color: #2d2d3a;
            color: #ffffff;
            border: 1px solid #4d4d5a;
        }
        
        QMenu::item:selected {
            background-color: #00d4ff;
            color: #000000;
        }
        
        /* Status Bar */
        QStatusBar {
            background-color: #2d2d3a;
            color: #ffffff;
            border-top: 1px solid #4d4d5a;
        }
        
        /* Buttons */
        QPushButton {
            background-color: #3d3d4a;
            color: #ffffff;
            border: 1px solid #4d4d5a;
            border-radius: 3px;
            padding: 5px 10px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #4d4d5a;
        }
        
        QPushButton:pressed {
            background-color: #00d4ff;
            color: #000000;
        }
        
        QPushButton:disabled {
            background-color: #2d2d3a;
            color: #808080;
        }
        
        /* Emergency Button */
        QPushButton#emergencyButton {
            background-color: #ff4757;
            color: #ffffff;
            font-weight: bold;
        }
        
        QPushButton#emergencyButton:hover {
            background-color: #ff3742;
        }
        
        /* Success Button */
        QPushButton#startButton {
            background-color: #00ff88;
            color: #000000;
        }
        
        QPushButton#startButton:hover {
            background-color: #00e676;
        }
        
        /* Labels */
        QLabel {
            color: #ffffff;
        }
        
        /* Progress Bar */
        QProgressBar {
            border: 1px solid #4d4d5a;
            border-radius: 3px;
            text-align: center;
            background-color: #2d2d3a;
        }
        
        QProgressBar::chunk {
            background-color: #00d4ff;
            border-radius: 2px;
        }
        
        /* Combo Box */
        QComboBox {
            background-color: #3d3d4a;
            color: #ffffff;
            border: 1px solid #4d4d5a;
            border-radius: 3px;
            padding: 3px 5px;
        }
        
        QComboBox:drop-down {
            border: none;
        }
        
        QComboBox QAbstractItemView {
            background-color: #2d2d3a;
            color: #ffffff;
            selection-background-color: #00d4ff;
        }
    )";

    app.setStyleSheet(darkStyleSheet);
    qInfo(gui) << "Applied dark theme stylesheet";
}

/**
 * @brief Show splash screen during application startup
 */
QSplashScreen* showSplashScreen()
{
    // Create a simple splash screen
    QPixmap splash(400, 300);
    splash.fill(QColor(30, 30, 46));  // Dark blue background
    
    QSplashScreen *splashScreen = new QSplashScreen(splash);
    splashScreen->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SplashScreen);
    splashScreen->showMessage("Loading Master Mind Trading System...", 
                             Qt::AlignBottom | Qt::AlignCenter, 
                             Qt::white);
    splashScreen->show();
    
    return splashScreen;
}

/**
 * @brief Handle uncaught exceptions
 */
void handleException(const std::exception& e)
{
    QString errorMsg = QString("Critical Error: %1").arg(e.what());
    qCritical(gui) << errorMsg;
    
    QMessageBox::critical(nullptr, "Critical Error", 
                         errorMsg + "\n\nThe application will now exit.");
}

/**
 * @brief Main application entry point
 */
int main(int argc, char *argv[])
{
    try {
        // Create Qt application
        QApplication app(argc, argv);
        
        // Initialize logging
        QLoggingCategory::setFilterRules("mastermind.*=true");
        
        qInfo(gui) << "Starting Master Mind Trading System GUI v1.0.0";
        qInfo(gui) << "Qt Version:" << QT_VERSION_STR;
        qInfo(gui) << "System:" << QSysInfo::prettyProductName();
        
        // Setup application style and properties
        setupApplicationStyle(app);
        
        // Show splash screen
        QSplashScreen *splash = showSplashScreen();
        app.processEvents();
        
        // Initialize application environment
        splash->showMessage("Initializing application environment...", 
                           Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        if (!initializeApplicationEnvironment()) {
            QMessageBox::critical(nullptr, "Initialization Error",
                                "Failed to initialize application environment.\n"
                                "Please check permissions and try again.");
            return -1;
        }
        
        // Small delay to show splash
        QThread::msleep(1000);
        
        // Initialize main window
        splash->showMessage("Loading main interface...", 
                           Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        MainWindow mainWindow;
        
        // Initialize trading system
        splash->showMessage("Initializing trading engine...", 
                           Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        mainWindow.initializeTrading();
        
        // Show main window
        splash->showMessage("Ready!", 
                           Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        QThread::msleep(500);
        splash->finish(&mainWindow);
        delete splash;
        
        mainWindow.show();
        mainWindow.restoreWindowState();
        
        qInfo(gui) << "Master Mind Trading System started successfully";
        
        // Run application event loop
        int result = app.exec();
        
        // Cleanup
        mainWindow.shutdownTrading();
        qInfo(gui) << "Application shutdown complete";
        
        return result;
        
    } catch (const std::exception& e) {
        handleException(e);
        return -1;
    } catch (...) {
        qCritical(gui) << "Unknown exception occurred during startup";
        QMessageBox::critical(nullptr, "Unknown Error",
                             "An unknown error occurred during startup.\n"
                             "The application will now exit.");
        return -1;
    }
}

// Include the MOC file for Q_OBJECT macro processing
#include "main_gui.moc" 