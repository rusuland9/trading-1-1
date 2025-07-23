#ifndef MASTERMIND_RISK_MANAGEMENT_WIDGET_H
#define MASTERMIND_RISK_MANAGEMENT_WIDGET_H

#include <QtWidgets/QWidget>

class QProgressBar;
class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

namespace MasterMind {

/**
 * @brief Widget for risk monitoring and controls
 * 
 * Displays real-time risk metrics, account information, drawdown tracking,
 * emergency controls, and risk event logging as specified in the requirements.
 */
class RiskManagementWidget : public QWidget {
    Q_OBJECT

public:
    explicit RiskManagementWidget(QWidget* parent = nullptr);
    ~RiskManagementWidget();

    // Public interface for updating risk data
    void setRiskParameters(double dailyRiskPercent, double maxDrawdownPercent, int consecutiveLossLimit);
    void updateCounterProgress(int currentOrders, int maxOrders);
    void updateConsecutiveLosses(int losses);
    void addRiskEvent(const QString& severity, const QString& description);

signals:
    void emergencyStopActivated();
    void emergencyStopDeactivated();
    void paperModeChanged(bool enabled);
    void countersReset();
    void counterCompleted();
    void consecutiveLossLimitReached();
    void riskMetricsUpdated();

private slots:
    void updateRiskMetrics();
    void onEmergencyStop();
    void onResetCounters();
    void onPaperModeToggled(bool enabled);

private:
    void setupUI();
    QWidget* createAccountInfoSection();
    QWidget* createRiskMetricsSection();
    QWidget* createCounterSystemSection();
    QWidget* createControlsSection();
    QWidget* createRiskEventsSection();
    void setupRiskEventsTable();
    void connectSignals();
    void initializeData();
    void updateAccountInfo();
    void addSampleRiskEvents();

    // UI components
    QProgressBar* dailyRiskProgressBar_;
    QProgressBar* drawdownProgressBar_;
    QLabel* equityLabel_;
    QLabel* balanceLabel_;
    QLabel* freeMarginLabel_;
    QLabel* dailyPnLLabel_;
    QLabel* currentDrawdownLabel_;
    QLabel* maxDrawdownLabel_;
    QLabel* consecutiveLossesLabel_;
    QProgressBar* counterProgressBar_;
    QPushButton* paperModeIndicator_;
    QPushButton* emergencyStopButton_;
    QPushButton* resetCountersButton_;
    QTableWidget* riskEventsTable_;
    QTimer* updateTimer_;

    // Data
    double currentEquity_;
    double currentBalance_;
    double freeMargin_;
    double dailyPnL_;
    double currentDrawdown_;
    double maxDrawdown_;
    int consecutiveLosses_;
    int currentCounterProgress_;
    int maxCounterSize_;
    bool isPaperMode_;
    bool isEmergencyStop_;
};

} // namespace MasterMind

#endif // MASTERMIND_RISK_MANAGEMENT_WIDGET_H 