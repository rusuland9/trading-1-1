#include "ui/RiskManagementWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QFont>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <iostream>

namespace MasterMind {

RiskManagementWidget::RiskManagementWidget(QWidget* parent)
    : QWidget(parent)
    , dailyRiskProgressBar_(nullptr)
    , drawdownProgressBar_(nullptr)
    , equityLabel_(nullptr)
    , balanceLabel_(nullptr)
    , freeMarginLabel_(nullptr)
    , dailyPnLLabel_(nullptr)
    , currentDrawdownLabel_(nullptr)
    , maxDrawdownLabel_(nullptr)
    , consecutiveLossesLabel_(nullptr)
    , counterProgressBar_(nullptr)
    , paperModeIndicator_(nullptr)
    , emergencyStopButton_(nullptr)
    , resetCountersButton_(nullptr)
    , riskEventsTable_(nullptr)
    , updateTimer_(nullptr)
    , currentEquity_(10000.0)
    , currentBalance_(10000.0)
    , freeMargin_(10000.0)
    , dailyPnL_(0.0)
    , currentDrawdown_(0.0)
    , maxDrawdown_(0.0)
    , consecutiveLosses_(0)
    , currentCounterProgress_(0)
    , maxCounterSize_(10)
    , isPaperMode_(true)
    , isEmergencyStop_(false)
{
    setupUI();
    connectSignals();
    initializeData();
    
    std::cout << "RiskManagementWidget initialized" << std::endl;
}

RiskManagementWidget::~RiskManagementWidget() = default;

void RiskManagementWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Title
    auto* titleLabel = new QLabel("Risk Management");
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Create sections
    layout->addWidget(createAccountInfoSection());
    layout->addWidget(createRiskMetricsSection());
    layout->addWidget(createCounterSystemSection());
    layout->addWidget(createControlsSection());
    layout->addWidget(createRiskEventsSection());
    
    // Apply styling
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 2px solid #4d4d5a;
            border-radius: 5px;
            margin: 10px 0px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        QProgressBar {
            border: 2px solid #4d4d5a;
            border-radius: 5px;
            text-align: center;
            font-weight: bold;
        }
        QProgressBar::chunk {
            background-color: #00d4ff;
            border-radius: 3px;
        }
        QProgressBar[status="warning"]::chunk {
            background-color: #ffb800;
        }
        QProgressBar[status="danger"]::chunk {
            background-color: #ff4757;
        }
        QPushButton#emergencyButton {
            background-color: #ff4757;
            color: #ffffff;
            font-weight: bold;
            font-size: 14px;
            min-height: 40px;
        }
        QPushButton#emergencyButton:pressed {
            background-color: #ff3742;
        }
        QPushButton#paperModeButton {
            background-color: #ffb800;
            color: #000000;
            font-weight: bold;
        }
        QLCDNumber {
            background-color: #1e1e2e;
            color: #00d4ff;
            border: 2px solid #4d4d5a;
            border-radius: 5px;
        }
    )");
}

QWidget* RiskManagementWidget::createAccountInfoSection() {
    auto* groupBox = new QGroupBox("Account Information");
    auto* layout = new QGridLayout(groupBox);
    
    int row = 0;
    
    // Equity
    layout->addWidget(new QLabel("Equity:"), row, 0);
    equityLabel_ = new QLabel("$10,000.00");
    equityLabel_->setStyleSheet("font-weight: bold; font-size: 14px; color: #00d4ff;");
    layout->addWidget(equityLabel_, row, 1);
    
    // Balance
    layout->addWidget(new QLabel("Balance:"), row, 2);
    balanceLabel_ = new QLabel("$10,000.00");
    balanceLabel_->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(balanceLabel_, row, 3);
    
    row++;
    
    // Free Margin
    layout->addWidget(new QLabel("Free Margin:"), row, 0);
    freeMarginLabel_ = new QLabel("$10,000.00");
    freeMarginLabel_->setStyleSheet("font-weight: bold; font-size: 14px; color: #00ff88;");
    layout->addWidget(freeMarginLabel_, row, 1);
    
    // Daily P&L
    layout->addWidget(new QLabel("Daily P&L:"), row, 2);
    dailyPnLLabel_ = new QLabel("$0.00");
    dailyPnLLabel_->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(dailyPnLLabel_, row, 3);
    
    return groupBox;
}

QWidget* RiskManagementWidget::createRiskMetricsSection() {
    auto* groupBox = new QGroupBox("Risk Metrics");
    auto* layout = new QGridLayout(groupBox);
    
    int row = 0;
    
    // Daily Risk Usage
    layout->addWidget(new QLabel("Daily Risk Usage:"), row, 0);
    dailyRiskProgressBar_ = new QProgressBar();
    dailyRiskProgressBar_->setRange(0, 100);
    dailyRiskProgressBar_->setValue(25);
    dailyRiskProgressBar_->setFormat("25% (2.0% limit)");
    layout->addWidget(dailyRiskProgressBar_, row, 1, 1, 2);
    
    row++;
    
    // Drawdown
    layout->addWidget(new QLabel("Current Drawdown:"), row, 0);
    drawdownProgressBar_ = new QProgressBar();
    drawdownProgressBar_->setRange(0, 100);
    drawdownProgressBar_->setValue(15);
    drawdownProgressBar_->setFormat("1.5% (5.0% limit)");
    layout->addWidget(drawdownProgressBar_, row, 1, 1, 2);
    
    row++;
    
    // Drawdown values
    layout->addWidget(new QLabel("Current Drawdown:"), row, 0);
    currentDrawdownLabel_ = new QLabel("1.5%");
    currentDrawdownLabel_->setStyleSheet("font-weight: bold; color: #ffb800;");
    layout->addWidget(currentDrawdownLabel_, row, 1);
    
    layout->addWidget(new QLabel("Max Drawdown:"), row, 2);
    maxDrawdownLabel_ = new QLabel("2.8%");
    maxDrawdownLabel_->setStyleSheet("font-weight: bold; color: #ff4757;");
    layout->addWidget(maxDrawdownLabel_, row, 3);
    
    row++;
    
    // Consecutive losses
    layout->addWidget(new QLabel("Consecutive Losses:"), row, 0);
    consecutiveLossesLabel_ = new QLabel("1 / 2 limit");
    consecutiveLossesLabel_->setStyleSheet("font-weight: bold;");
    layout->addWidget(consecutiveLossesLabel_, row, 1);
    
    return groupBox;
}

QWidget* RiskManagementWidget::createCounterSystemSection() {
    auto* groupBox = new QGroupBox("Counter System");
    auto* layout = new QGridLayout(groupBox);
    
    int row = 0;
    
    // Counter progress
    layout->addWidget(new QLabel("Current Counter:"), row, 0);
    counterProgressBar_ = new QProgressBar();
    counterProgressBar_->setRange(0, maxCounterSize_);
    counterProgressBar_->setValue(currentCounterProgress_);
    counterProgressBar_->setFormat(QString("%1 / %2 orders").arg(currentCounterProgress_).arg(maxCounterSize_));
    layout->addWidget(counterProgressBar_, row, 1, 1, 2);
    
    row++;
    
    // Counter stats using LCD numbers for a more professional look
    layout->addWidget(new QLabel("Counter #:"), row, 0);
    auto* counterNumberLCD = new QLCDNumber(2);
    counterNumberLCD->display(5);
    counterNumberLCD->setMaximumHeight(40);
    layout->addWidget(counterNumberLCD, row, 1);
    
    layout->addWidget(new QLabel("Counter P&L:"), row, 2);
    auto* counterPnLLCD = new QLCDNumber(6);
    counterPnLLCD->display(125.50);
    counterPnLLCD->setMaximumHeight(40);
    layout->addWidget(counterPnLLCD, row, 3);
    
    return groupBox;
}

QWidget* RiskManagementWidget::createControlsSection() {
    auto* groupBox = new QGroupBox("Risk Controls");
    auto* layout = new QHBoxLayout(groupBox);
    
    // Paper mode indicator
    paperModeIndicator_ = new QPushButton("PAPER MODE");
    paperModeIndicator_->setObjectName("paperModeButton");
    paperModeIndicator_->setCheckable(true);
    paperModeIndicator_->setChecked(isPaperMode_);
    paperModeIndicator_->setMinimumHeight(40);
    layout->addWidget(paperModeIndicator_);
    
    // Emergency stop button
    emergencyStopButton_ = new QPushButton("EMERGENCY STOP");
    emergencyStopButton_->setObjectName("emergencyButton");
    layout->addWidget(emergencyStopButton_);
    
    // Reset counters button
    resetCountersButton_ = new QPushButton("Reset Counters");
    resetCountersButton_->setMinimumHeight(40);
    layout->addWidget(resetCountersButton_);
    
    // Risk settings button
    auto* riskSettingsButton = new QPushButton("Risk Settings");
    riskSettingsButton->setMinimumHeight(40);
    layout->addWidget(riskSettingsButton);
    
    return groupBox;
}

QWidget* RiskManagementWidget::createRiskEventsSection() {
    auto* groupBox = new QGroupBox("Risk Events Log");
    auto* layout = new QVBoxLayout(groupBox);
    
    // Risk events table
    riskEventsTable_ = new QTableWidget();
    setupRiskEventsTable();
    layout->addWidget(riskEventsTable_);
    
    return groupBox;
}

void RiskManagementWidget::setupRiskEventsTable() {
    QStringList headers = {"Time", "Event", "Severity", "Details"};
    riskEventsTable_->setColumnCount(headers.size());
    riskEventsTable_->setHorizontalHeaderLabels(headers);
    
    riskEventsTable_->setAlternatingRowColors(true);
    riskEventsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    riskEventsTable_->setMaximumHeight(150);
    
    // Set column widths
    riskEventsTable_->setColumnWidth(0, 80);  // Time
    riskEventsTable_->setColumnWidth(1, 150); // Event
    riskEventsTable_->setColumnWidth(2, 80);  // Severity
    
    riskEventsTable_->horizontalHeader()->setStretchLastSection(true);
    riskEventsTable_->verticalHeader()->setVisible(false);
    
    // Add sample risk events
    addSampleRiskEvents();
}

void RiskManagementWidget::connectSignals() {
    // Setup update timer
    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(1000); // 1 second updates
    connect(updateTimer_, &QTimer::timeout, this, &RiskManagementWidget::updateRiskMetrics);
    updateTimer_->start();
    
    // Connect control buttons
    connect(emergencyStopButton_, &QPushButton::clicked, this, &RiskManagementWidget::onEmergencyStop);
    connect(resetCountersButton_, &QPushButton::clicked, this, &RiskManagementWidget::onResetCounters);
    connect(paperModeIndicator_, &QPushButton::toggled, this, &RiskManagementWidget::onPaperModeToggled);
}

void RiskManagementWidget::initializeData() {
    updateAccountInfo();
    updateRiskMetrics();
}

void RiskManagementWidget::addSampleRiskEvents() {
    QStringList sampleEvents = {
        "10:30:45|Risk Limit Warning|WARNING|Daily risk usage exceeded 80%",
        "10:25:12|Consecutive Loss|ALERT|2 consecutive losses detected",
        "09:45:33|Drawdown Alert|WARNING|Current drawdown: 3.2%",
        "09:15:20|Paper Mode Activated|INFO|Switched to paper trading mode"
    };
    
    riskEventsTable_->setRowCount(sampleEvents.size());
    
    for (int i = 0; i < sampleEvents.size(); ++i) {
        QStringList eventData = sampleEvents[i].split("|");
        for (int j = 0; j < eventData.size(); ++j) {
            auto* item = new QTableWidgetItem(eventData[j]);
            
            // Color code severity
            if (j == 2) { // Severity column
                if (eventData[j] == "WARNING") {
                    item->setForeground(QColor(255, 184, 0)); // Orange
                } else if (eventData[j] == "ALERT") {
                    item->setForeground(QColor(255, 71, 87)); // Red
                } else if (eventData[j] == "INFO") {
                    item->setForeground(QColor(0, 255, 136)); // Green
                }
            }
            
            riskEventsTable_->setItem(i, j, item);
        }
    }
}

void RiskManagementWidget::updateAccountInfo() {
    // Simulate account info updates
    equityLabel_->setText(QString("$%1").arg(currentEquity_, 0, 'f', 2));
    balanceLabel_->setText(QString("$%1").arg(currentBalance_, 0, 'f', 2));
    freeMarginLabel_->setText(QString("$%1").arg(freeMargin_, 0, 'f', 2));
    
    // Update daily P&L color
    if (dailyPnL_ > 0) {
        dailyPnLLabel_->setStyleSheet("font-weight: bold; font-size: 14px; color: #00ff88;");
    } else if (dailyPnL_ < 0) {
        dailyPnLLabel_->setStyleSheet("font-weight: bold; font-size: 14px; color: #ff4757;");
    } else {
        dailyPnLLabel_->setStyleSheet("font-weight: bold; font-size: 14px;");
    }
    dailyPnLLabel_->setText(QString("$%1").arg(dailyPnL_, 0, 'f', 2));
}

void RiskManagementWidget::updateRiskMetrics() {
    // Simulate risk metrics updates
    currentDrawdown_ += ((qrand() % 10) - 5) * 0.01; // Random walk
    currentDrawdown_ = std::max(0.0, std::min(currentDrawdown_, 10.0));
    
    maxDrawdown_ = std::max(maxDrawdown_, currentDrawdown_);
    
    // Update drawdown progress bar
    int drawdownPercent = static_cast<int>(currentDrawdown_ * 20); // Scale to 0-100 for 0-5% range
    drawdownProgressBar_->setValue(drawdownPercent);
    drawdownProgressBar_->setFormat(QString("%1% (5.0% limit)").arg(currentDrawdown_, 0, 'f', 1));
    
    // Set color based on risk level
    if (currentDrawdown_ > 4.0) {
        drawdownProgressBar_->setProperty("status", "danger");
    } else if (currentDrawdown_ > 2.5) {
        drawdownProgressBar_->setProperty("status", "warning");
    } else {
        drawdownProgressBar_->setProperty("status", "normal");
    }
    drawdownProgressBar_->style()->unpolish(drawdownProgressBar_);
    drawdownProgressBar_->style()->polish(drawdownProgressBar_);
    
    // Update labels
    currentDrawdownLabel_->setText(QString("%1%").arg(currentDrawdown_, 0, 'f', 1));
    maxDrawdownLabel_->setText(QString("%1%").arg(maxDrawdown_, 0, 'f', 1));
    consecutiveLossesLabel_->setText(QString("%1 / 2 limit").arg(consecutiveLosses_));
    
    // Update counter progress
    counterProgressBar_->setValue(currentCounterProgress_);
    counterProgressBar_->setFormat(QString("%1 / %2 orders").arg(currentCounterProgress_).arg(maxCounterSize_));
    
    // Simulate account updates
    dailyPnL_ += ((qrand() % 20) - 10) * 0.5; // Random P&L changes
    currentEquity_ = currentBalance_ + dailyPnL_;
    
    updateAccountInfo();
    
    // Emit signal for other components
    emit riskMetricsUpdated();
}

void RiskManagementWidget::onEmergencyStop() {
    if (isEmergencyStop_) {
        // Deactivate emergency stop
        isEmergencyStop_ = false;
        emergencyStopButton_->setText("EMERGENCY STOP");
        emergencyStopButton_->setStyleSheet("");
        
        QMessageBox::information(this, "Emergency Stop", "Emergency stop has been deactivated.");
        emit emergencyStopDeactivated();
    } else {
        // Activate emergency stop
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Emergency Stop",
            "Are you sure you want to activate emergency stop?\n\nThis will:\n"
            "- Stop all trading immediately\n"
            "- Cancel all active orders\n"
            "- Close all open positions\n"
            "- Switch to paper trading mode",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            isEmergencyStop_ = true;
            emergencyStopButton_->setText("STOP ACTIVE");
            emergencyStopButton_->setStyleSheet("background-color: #ff3742; animation: blink 1s infinite;");
            
            // Add risk event
            addRiskEvent("EMERGENCY", "Emergency stop activated by user");
            
            emit emergencyStopActivated();
        }
    }
}

void RiskManagementWidget::onResetCounters() {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Reset Counters",
        "Are you sure you want to reset all trading counters?\n\n"
        "This will reset:\n"
        "- Current counter progress\n"
        "- Daily P&L\n"
        "- Risk usage statistics",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        currentCounterProgress_ = 0;
        dailyPnL_ = 0.0;
        consecutiveLosses_ = 0;
        
        // Reset progress bars
        dailyRiskProgressBar_->setValue(0);
        counterProgressBar_->setValue(0);
        
        addRiskEvent("INFO", "Trading counters reset by user");
        emit countersReset();
        
        QMessageBox::information(this, "Counters Reset", "All trading counters have been reset.");
    }
}

void RiskManagementWidget::onPaperModeToggled(bool enabled) {
    isPaperMode_ = enabled;
    
    if (enabled) {
        paperModeIndicator_->setText("PAPER MODE");
        paperModeIndicator_->setStyleSheet("background-color: #ffb800; color: #000000; font-weight: bold;");
        addRiskEvent("INFO", "Switched to paper trading mode");
    } else {
        paperModeIndicator_->setText("LIVE MODE");
        paperModeIndicator_->setStyleSheet("background-color: #00ff88; color: #000000; font-weight: bold;");
        addRiskEvent("WARNING", "Switched to live trading mode");
    }
    
    emit paperModeChanged(enabled);
}

void RiskManagementWidget::addRiskEvent(const QString& severity, const QString& description) {
    // Add new risk event to the table
    int newRow = riskEventsTable_->rowCount();
    riskEventsTable_->insertRow(0); // Insert at top
    
    // Set event data
    riskEventsTable_->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
    riskEventsTable_->setItem(0, 1, new QTableWidgetItem("Risk Event"));
    
    auto* severityItem = new QTableWidgetItem(severity);
    if (severity == "WARNING") {
        severityItem->setForeground(QColor(255, 184, 0)); // Orange
    } else if (severity == "EMERGENCY") {
        severityItem->setForeground(QColor(255, 71, 87)); // Red
    } else if (severity == "INFO") {
        severityItem->setForeground(QColor(0, 255, 136)); // Green
    }
    riskEventsTable_->setItem(0, 2, severityItem);
    riskEventsTable_->setItem(0, 3, new QTableWidgetItem(description));
    
    // Limit table size
    if (riskEventsTable_->rowCount() > 50) {
        riskEventsTable_->removeRow(riskEventsTable_->rowCount() - 1);
    }
    
    std::cout << "Risk event: " << severity.toStdString() << " - " << description.toStdString() << std::endl;
}

void RiskManagementWidget::setRiskParameters(double dailyRiskPercent, double maxDrawdownPercent, int consecutiveLossLimit) {
    // Update risk parameters
    dailyRiskProgressBar_->setFormat(QString("%1% (%2% limit)").arg(25).arg(dailyRiskPercent * 100, 0, 'f', 1));
    drawdownProgressBar_->setFormat(QString("%1% (%2% limit)").arg(currentDrawdown_, 0, 'f', 1).arg(maxDrawdownPercent * 100, 0, 'f', 1));
    consecutiveLossesLabel_->setText(QString("%1 / %2 limit").arg(consecutiveLosses_).arg(consecutiveLossLimit));
    
    addRiskEvent("INFO", QString("Risk parameters updated: Daily: %1%, Drawdown: %2%, Loss limit: %3")
                        .arg(dailyRiskPercent * 100, 0, 'f', 1)
                        .arg(maxDrawdownPercent * 100, 0, 'f', 1)
                        .arg(consecutiveLossLimit));
}

void RiskManagementWidget::updateCounterProgress(int currentOrders, int maxOrders) {
    currentCounterProgress_ = currentOrders;
    maxCounterSize_ = maxOrders;
    
    counterProgressBar_->setMaximum(maxOrders);
    counterProgressBar_->setValue(currentOrders);
    counterProgressBar_->setFormat(QString("%1 / %2 orders").arg(currentOrders).arg(maxOrders));
    
    // Check if counter is complete
    if (currentOrders >= maxOrders) {
        addRiskEvent("INFO", QString("Counter completed: %1 orders").arg(maxOrders));
        emit counterCompleted();
    }
}

void RiskManagementWidget::updateConsecutiveLosses(int losses) {
    consecutiveLosses_ = losses;
    consecutiveLossesLabel_->setText(QString("%1 / 2 limit").arg(losses));
    
    if (losses >= 2) {
        addRiskEvent("WARNING", QString("Consecutive loss limit reached: %1").arg(losses));
        emit consecutiveLossLimitReached();
    }
}

} // namespace MasterMind 