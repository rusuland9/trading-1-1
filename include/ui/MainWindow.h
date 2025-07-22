#ifndef MASTERMIND_MAINWINDOW_H
#define MASTERMIND_MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QProgressBar>
#include <QtCore/QTimer>
#include <QtCore/QSettings>
#include <memory>

// Forward declarations
namespace MasterMind {
class TradingEngine;
}

class TradingDashboard;
class RenkoChartWidget;
class MarketDataWidget;
class OrderManagementWidget;
class RiskManagementWidget;
class PositionWidget;
class LogWidget;
class PatternDetectionWidget;
class PerformanceWidget;
class ExchangeStatusWidget;
class ConfigurationWidget;

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
QT_END_NAMESPACE

/**
 * @brief Main application window for Master Mind Trading System
 * 
 * Central hub that coordinates all trading interface components including:
 * - Real-time Renko charts with pattern detection
 * - Order management and execution
 * - Risk monitoring and controls
 * - Multi-exchange integration status
 * - Performance analytics and reporting
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Application lifecycle
    void initializeTrading();
    void shutdownTrading();
    bool isInitialized() const;

    // UI state management
    void saveWindowState();
    void restoreWindowState();
    void resetLayout();

public slots:
    // Menu actions
    void onNewConfiguration();
    void onOpenConfiguration();
    void onSaveConfiguration();
    void onExportReport();
    void onExit();

    // View actions
    void onToggleFullscreen();
    void onResetLayout();
    void onToggleTheme();

    // Trading actions
    void onStartTrading();
    void onStopTrading();
    void onPaperMode(bool enable);
    void onEmergencyStop();

    // Risk actions
    void onRiskSettings();
    void onResetCounters();

    // Help actions
    void onAbout();
    void onUserManual();

    // System status updates
    void updateConnectionStatus();
    void updateTradingStatus();
    void updateRiskStatus();

signals:
    // System signals
    void tradingStarted();
    void tradingStopped();
    void emergencyStopTriggered();
    void configurationChanged();

protected:
    // Qt event handlers
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // Internal update timers
    void updateStatusBar();
    void updatePerformanceMetrics();
    void checkSystemHealth();

private:
    // UI initialization
    void createActions();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void createDockWidgets();
    void createCentralWidget();
    void setupLayoutStyle();
    void applyDarkTheme();
    void applyLightTheme();

    // Widget management
    void connectWidgetSignals();
    void updateWidgetStates();
    void refreshAllWidgets();

    // Settings management
    void loadSettings();
    void saveSettings();
    bool confirmClose();

    // Trading system integration
    std::unique_ptr<MasterMind::TradingEngine> tradingEngine_;
    bool initialized_;
    bool tradingActive_;

    // Central widget and dashboard
    TradingDashboard *centralDashboard_;

    // Main dock widgets
    QDockWidget *marketDataDock_;
    QDockWidget *orderManagementDock_;
    QDockWidget *riskManagementDock_;
    QDockWidget *positionDock_;
    QDockWidget *logDock_;
    QDockWidget *patternDetectionDock_;
    QDockWidget *performanceDock_;
    QDockWidget *exchangeStatusDock_;

    // Widget instances
    RenkoChartWidget *renkoChartWidget_;
    MarketDataWidget *marketDataWidget_;
    OrderManagementWidget *orderManagementWidget_;
    RiskManagementWidget *riskManagementWidget_;
    PositionWidget *positionWidget_;
    LogWidget *logWidget_;
    PatternDetectionWidget *patternDetectionWidget_;
    PerformanceWidget *performanceWidget_;
    ExchangeStatusWidget *exchangeStatusWidget_;
    ConfigurationWidget *configurationWidget_;

    // Menu bar
    QMenuBar *menuBar_;
    
    // Menus
    QMenu *fileMenu_;
    QMenu *viewMenu_;
    QMenu *tradingMenu_;
    QMenu *riskMenu_;
    QMenu *toolsMenu_;
    QMenu *helpMenu_;

    // File menu actions
    QAction *newConfigAction_;
    QAction *openConfigAction_;
    QAction *saveConfigAction_;
    QAction *exportReportAction_;
    QAction *exitAction_;

    // View menu actions
    QAction *fullscreenAction_;
    QAction *resetLayoutAction_;
    QAction *darkThemeAction_;
    QAction *lightThemeAction_;
    QActionGroup *themeActionGroup_;

    // Trading menu actions
    QAction *startTradingAction_;
    QAction *stopTradingAction_;
    QAction *paperModeAction_;
    QAction *emergencyStopAction_;

    // Risk menu actions
    QAction *riskSettingsAction_;
    QAction *resetCountersAction_;

    // Tools menu actions
    QAction *configurationAction_;

    // Help menu actions
    QAction *aboutAction_;
    QAction *userManualAction_;

    // Toolbar
    QToolBar *mainToolBar_;
    QPushButton *startTradingButton_;
    QPushButton *stopTradingButton_;
    QPushButton *emergencyButton_;
    QComboBox *symbolSelector_;
    QPushButton *paperModeButton_;

    // Status bar
    QStatusBar *statusBar_;
    QLabel *connectionStatusLabel_;
    QLabel *tradingStatusLabel_;
    QLabel *riskStatusLabel_;
    QLabel *timeLabel_;
    QProgressBar *riskProgressBar_;

    // System timers
    QTimer *statusUpdateTimer_;
    QTimer *performanceUpdateTimer_;
    QTimer *healthCheckTimer_;

    // UI state
    QSettings *settings_;
    bool isDarkTheme_;
    bool isFullscreen_;

    // Performance tracking
    QTimer systemStartTime_;
    int totalTrades_;
    double totalPnL_;

    // Constants
    static const int STATUS_UPDATE_INTERVAL = 1000;  // 1 second
    static const int PERFORMANCE_UPDATE_INTERVAL = 5000;  // 5 seconds
    static const int HEALTH_CHECK_INTERVAL = 10000;  // 10 seconds
    static const QString ORGANIZATION_NAME;
    static const QString APPLICATION_NAME;
    static const QString SETTINGS_GEOMETRY;
    static const QString SETTINGS_WINDOWSTATE;
    static const QString SETTINGS_THEME;
};

#endif // MASTERMIND_MAINWINDOW_H 