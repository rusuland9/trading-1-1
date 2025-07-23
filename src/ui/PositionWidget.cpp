#include "ui/PositionWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QFont>
#include <iostream>

namespace MasterMind {

PositionWidget::PositionWidget(QWidget* parent)
    : QWidget(parent)
    , positionsTable_(nullptr)
    , totalPnLLabel_(nullptr)
    , totalExposureLabel_(nullptr)
    , openPositionsLabel_(nullptr)
    , closeAllButton_(nullptr)
    , refreshButton_(nullptr)
    , updateTimer_(nullptr)
    , totalPnL_(0.0)
    , totalExposure_(0.0)
    , openPositionsCount_(0)
{
    setupUI();
    connectSignals();
    initializeData();
    
    std::cout << "PositionWidget initialized" << std::endl;
}

PositionWidget::~PositionWidget() = default;

void PositionWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Title
    auto* titleLabel = new QLabel("Position Management");
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Create summary section
    layout->addWidget(createSummarySection());
    
    // Create positions table section
    layout->addWidget(createPositionsSection());
    
    // Create control buttons section
    layout->addWidget(createControlsSection());
    
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
        QTableWidget {
            alternate-background-color: #2d2d3a;
            background-color: #1e1e2e;
            gridline-color: #4d4d5a;
            color: #ffffff;
        }
        QHeaderView::section {
            background-color: #3d3d4a;
            color: #ffffff;
            border: 1px solid #4d4d5a;
            font-weight: bold;
        }
        QPushButton#closeAllButton {
            background-color: #ff4757;
            color: #ffffff;
            font-weight: bold;
        }
        QPushButton#closeAllButton:hover {
            background-color: #ff3742;
        }
    )");
}

QWidget* PositionWidget::createSummarySection() {
    auto* groupBox = new QGroupBox("Position Summary");
    auto* layout = new QHBoxLayout(groupBox);
    
    // Total P&L
    auto* pnlLayout = new QVBoxLayout();
    pnlLayout->addWidget(new QLabel("Total P&L:"));
    totalPnLLabel_ = new QLabel("$0.00");
    totalPnLLabel_->setStyleSheet("font-weight: bold; font-size: 16px; color: #00d4ff;");
    pnlLayout->addWidget(totalPnLLabel_);
    layout->addLayout(pnlLayout);
    
    // Total Exposure
    auto* exposureLayout = new QVBoxLayout();
    exposureLayout->addWidget(new QLabel("Total Exposure:"));
    totalExposureLabel_ = new QLabel("$0.00");
    totalExposureLabel_->setStyleSheet("font-weight: bold; font-size: 16px;");
    exposureLayout->addWidget(totalExposureLabel_);
    layout->addLayout(exposureLayout);
    
    // Open Positions Count
    auto* countLayout = new QVBoxLayout();
    countLayout->addWidget(new QLabel("Open Positions:"));
    openPositionsLabel_ = new QLabel("0");
    openPositionsLabel_->setStyleSheet("font-weight: bold; font-size: 16px;");
    countLayout->addWidget(openPositionsLabel_);
    layout->addLayout(countLayout);
    
    layout->addStretch();
    
    return groupBox;
}

QWidget* PositionWidget::createPositionsSection() {
    auto* groupBox = new QGroupBox("Open Positions");
    auto* layout = new QVBoxLayout(groupBox);
    
    // Positions table
    positionsTable_ = new QTableWidget();
    setupPositionsTable();
    layout->addWidget(positionsTable_);
    
    return groupBox;
}

QWidget* PositionWidget::createControlsSection() {
    auto* groupBox = new QGroupBox("Position Controls");
    auto* layout = new QHBoxLayout(groupBox);
    
    // Refresh button
    refreshButton_ = new QPushButton("Refresh");
    refreshButton_->setMinimumHeight(35);
    layout->addWidget(refreshButton_);
    
    // Close all button
    closeAllButton_ = new QPushButton("Close All Positions");
    closeAllButton_->setObjectName("closeAllButton");
    closeAllButton_->setMinimumHeight(35);
    layout->addWidget(closeAllButton_);
    
    layout->addStretch();
    
    return groupBox;
}

void PositionWidget::setupPositionsTable() {
    QStringList headers = {"Symbol", "Side", "Size", "Entry Price", "Current Price", "P&L", "P&L %", "Margin", "Time", "Actions"};
    positionsTable_->setColumnCount(headers.size());
    positionsTable_->setHorizontalHeaderLabels(headers);
    
    positionsTable_->setAlternatingRowColors(true);
    positionsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    positionsTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Set column widths
    positionsTable_->setColumnWidth(0, 80);  // Symbol
    positionsTable_->setColumnWidth(1, 60);  // Side
    positionsTable_->setColumnWidth(2, 80);  // Size
    positionsTable_->setColumnWidth(3, 90);  // Entry Price
    positionsTable_->setColumnWidth(4, 90);  // Current Price
    positionsTable_->setColumnWidth(5, 80);  // P&L
    positionsTable_->setColumnWidth(6, 70);  // P&L %
    positionsTable_->setColumnWidth(7, 80);  // Margin
    positionsTable_->setColumnWidth(8, 80);  // Time
    
    positionsTable_->horizontalHeader()->setStretchLastSection(true);
    positionsTable_->verticalHeader()->setVisible(false);
}

void PositionWidget::connectSignals() {
    // Setup update timer
    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(1000); // 1 second updates
    connect(updateTimer_, &QTimer::timeout, this, &PositionWidget::updatePositions);
    updateTimer_->start();
    
    // Connect control buttons
    connect(refreshButton_, &QPushButton::clicked, this, &PositionWidget::onRefreshPositions);
    connect(closeAllButton_, &QPushButton::clicked, this, &PositionWidget::onCloseAllPositions);
}

void PositionWidget::initializeData() {
    // Add some sample positions for demonstration
    addSamplePositions();
    updateSummary();
}

void PositionWidget::addSamplePositions() {
    // Sample positions for demonstration
    QStringList samplePositions = {
        "EURUSD|BUY|0.10|1.1050|1.1075|+2.50|+0.23%|55.25|10:30:45",
        "GBPUSD|SELL|0.20|1.2800|1.2785|-3.00|-0.12%|128.00|10:15:22",
        "BTCUSD|BUY|0.01|45000.0|45250.0|+25.00|+0.56%|450.00|09:45:12",
        "ETHUSD|BUY|0.05|3000.0|2950.0|-2.50|-0.17%|150.00|09:30:33"
    };
    
    positionsTable_->setRowCount(samplePositions.size());
    openPositionsCount_ = samplePositions.size();
    
    for (int i = 0; i < samplePositions.size(); ++i) {
        QStringList positionData = samplePositions[i].split("|");
        for (int j = 0; j < positionData.size(); ++j) {
            auto* item = new QTableWidgetItem(positionData[j]);
            
            // Right-align numeric columns
            if (j >= 2 && j <= 7) { // Size, Entry Price, Current Price, P&L, P&L %, Margin
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            }
            
            // Color code P&L
            if (j == 5) { // P&L column
                if (positionData[j].startsWith("+")) {
                    item->setForeground(QColor(0, 255, 136)); // Green for profit
                } else if (positionData[j].startsWith("-")) {
                    item->setForeground(QColor(255, 71, 87)); // Red for loss
                }
            }
            
            // Color code P&L %
            if (j == 6) { // P&L % column
                if (positionData[j].startsWith("+")) {
                    item->setForeground(QColor(0, 255, 136)); // Green for profit
                } else if (positionData[j].startsWith("-")) {
                    item->setForeground(QColor(255, 71, 87)); // Red for loss
                }
            }
            
            // Color code side
            if (j == 1) { // Side column
                if (positionData[j] == "BUY") {
                    item->setForeground(QColor(0, 255, 136)); // Green for buy
                } else if (positionData[j] == "SELL") {
                    item->setForeground(QColor(255, 71, 87)); // Red for sell
                }
            }
            
            positionsTable_->setItem(i, j, item);
        }
        
        // Add close button in actions column
        auto* closeButton = new QPushButton("Close");
        closeButton->setMaximumHeight(25);
        closeButton->setStyleSheet("background-color: #ff4757; color: white; font-weight: bold;");
        connect(closeButton, &QPushButton::clicked, [this, i]() { 
            closePosition(i); 
        });
        positionsTable_->setCellWidget(i, 9, closeButton);
    }
    
    // Calculate sample totals
    totalPnL_ = 22.0; // Net P&L from sample data
    totalExposure_ = 783.25; // Total margin used
}

void PositionWidget::updateSummary() {
    // Update summary labels
    openPositionsLabel_->setText(QString::number(openPositionsCount_));
    totalExposureLabel_->setText(QString("$%1").arg(totalExposure_, 0, 'f', 2));
    
    // Update total P&L with color coding
    if (totalPnL_ > 0) {
        totalPnLLabel_->setStyleSheet("font-weight: bold; font-size: 16px; color: #00ff88;");
        totalPnLLabel_->setText(QString("+$%1").arg(totalPnL_, 0, 'f', 2));
    } else if (totalPnL_ < 0) {
        totalPnLLabel_->setStyleSheet("font-weight: bold; font-size: 16px; color: #ff4757;");
        totalPnLLabel_->setText(QString("-$%1").arg(std::abs(totalPnL_), 0, 'f', 2));
    } else {
        totalPnLLabel_->setStyleSheet("font-weight: bold; font-size: 16px; color: #00d4ff;");
        totalPnLLabel_->setText("$0.00");
    }
}

void PositionWidget::updatePositions() {
    // Simulate real-time position updates
    for (int i = 0; i < positionsTable_->rowCount(); ++i) {
        // Get current price item
        auto* currentPriceItem = positionsTable_->item(i, 4);
        auto* pnlItem = positionsTable_->item(i, 5);
        auto* pnlPercentItem = positionsTable_->item(i, 6);
        
        if (currentPriceItem && pnlItem && pnlPercentItem) {
            // Simulate price updates (small random changes)
            double currentPrice = currentPriceItem->text().toDouble();
            double entryPrice = positionsTable_->item(i, 3)->text().toDouble();
            QString side = positionsTable_->item(i, 1)->text();
            double size = positionsTable_->item(i, 2)->text().toDouble();
            
            // Small random price movement
            double priceChange = ((qrand() % 20) - 10) * 0.00001;
            if (positionsTable_->item(i, 0)->text().contains("BTC")) {
                priceChange *= 1000; // Larger movements for crypto
            } else if (positionsTable_->item(i, 0)->text().contains("ETH")) {
                priceChange *= 100;
            }
            
            currentPrice += priceChange;
            currentPriceItem->setText(QString::number(currentPrice, 'f', 5));
            
            // Calculate new P&L
            double pnl = 0.0;
            if (side == "BUY") {
                pnl = (currentPrice - entryPrice) * size * 100000; // Assuming forex units
            } else {
                pnl = (entryPrice - currentPrice) * size * 100000;
            }
            
            // Special handling for crypto
            if (positionsTable_->item(i, 0)->text().contains("BTC") || 
                positionsTable_->item(i, 0)->text().contains("ETH")) {
                if (side == "BUY") {
                    pnl = (currentPrice - entryPrice) * size;
                } else {
                    pnl = (entryPrice - currentPrice) * size;
                }
            }
            
            // Update P&L
            if (pnl > 0) {
                pnlItem->setText(QString("+%1").arg(pnl, 0, 'f', 2));
                pnlItem->setForeground(QColor(0, 255, 136));
            } else {
                pnlItem->setText(QString("%1").arg(pnl, 0, 'f', 2));
                pnlItem->setForeground(QColor(255, 71, 87));
            }
            
            // Update P&L %
            double pnlPercent = (pnl / (entryPrice * size * 100)) * 100;
            if (positionsTable_->item(i, 0)->text().contains("BTC") || 
                positionsTable_->item(i, 0)->text().contains("ETH")) {
                pnlPercent = (pnl / (entryPrice * size)) * 100;
            }
            
            if (pnlPercent > 0) {
                pnlPercentItem->setText(QString("+%1%").arg(pnlPercent, 0, 'f', 2));
                pnlPercentItem->setForeground(QColor(0, 255, 136));
            } else {
                pnlPercentItem->setText(QString("%1%").arg(pnlPercent, 0, 'f', 2));
                pnlPercentItem->setForeground(QColor(255, 71, 87));
            }
        }
    }
    
    // Recalculate total P&L
    recalculateTotalPnL();
    updateSummary();
    
    // Emit signal for other components
    emit positionsUpdated();
}

void PositionWidget::recalculateTotalPnL() {
    totalPnL_ = 0.0;
    
    for (int i = 0; i < positionsTable_->rowCount(); ++i) {
        auto* pnlItem = positionsTable_->item(i, 5);
        if (pnlItem) {
            QString pnlText = pnlItem->text();
            pnlText.remove("+").remove("$");
            totalPnL_ += pnlText.toDouble();
        }
    }
}

void PositionWidget::onRefreshPositions() {
    // Simulate refreshing positions from trading engine
    emit positionsRefreshRequested();
    std::cout << "Refreshing positions..." << std::endl;
    
    // Update display
    updatePositions();
}

void PositionWidget::onCloseAllPositions() {
    if (positionsTable_->rowCount() == 0) {
        QMessageBox::information(this, "No Positions", "There are no open positions to close.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Close All Positions",
        QString("Are you sure you want to close all %1 open positions?\n\n"
                "This action cannot be undone and may result in losses.")
                .arg(positionsTable_->rowCount()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Close all positions
        int closedCount = positionsTable_->rowCount();
        positionsTable_->setRowCount(0);
        openPositionsCount_ = 0;
        totalPnL_ = 0.0;
        totalExposure_ = 0.0;
        
        updateSummary();
        emit allPositionsClosed();
        
        QMessageBox::information(this, "Positions Closed", 
                                QString("All %1 positions have been closed.").arg(closedCount));
        
        std::cout << "All positions closed: " << closedCount << " positions" << std::endl;
    }
}

void PositionWidget::closePosition(int row) {
    if (row >= 0 && row < positionsTable_->rowCount()) {
        QString symbol = positionsTable_->item(row, 0)->text();
        QString side = positionsTable_->item(row, 1)->text();
        double size = positionsTable_->item(row, 2)->text().toDouble();
        
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Close Position",
            QString("Close position: %1 %2 %3?")
                .arg(size, 0, 'f', 2)
                .arg(symbol)
                .arg(side),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            // Remove position from table
            positionsTable_->removeRow(row);
            openPositionsCount_--;
            
            // Recalculate totals
            recalculateTotalPnL();
            updateSummary();
            
            emit positionClosed(symbol, side, size);
            
            std::cout << "Position closed: " << symbol.toStdString() 
                      << " " << side.toStdString() 
                      << " " << size << std::endl;
        }
    }
}

void PositionWidget::addPosition(const QString& symbol, const QString& side, double size, double entryPrice) {
    int newRow = positionsTable_->rowCount();
    positionsTable_->insertRow(newRow);
    
    // Set position data
    positionsTable_->setItem(newRow, 0, new QTableWidgetItem(symbol));
    
    auto* sideItem = new QTableWidgetItem(side);
    if (side == "BUY") {
        sideItem->setForeground(QColor(0, 255, 136));
    } else {
        sideItem->setForeground(QColor(255, 71, 87));
    }
    positionsTable_->setItem(newRow, 1, sideItem);
    
    auto* sizeItem = new QTableWidgetItem(QString::number(size, 'f', 2));
    sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionsTable_->setItem(newRow, 2, sizeItem);
    
    auto* entryPriceItem = new QTableWidgetItem(QString::number(entryPrice, 'f', 5));
    entryPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionsTable_->setItem(newRow, 3, entryPriceItem);
    
    auto* currentPriceItem = new QTableWidgetItem(QString::number(entryPrice, 'f', 5));
    currentPriceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionsTable_->setItem(newRow, 4, currentPriceItem);
    
    auto* pnlItem = new QTableWidgetItem("$0.00");
    pnlItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionsTable_->setItem(newRow, 5, pnlItem);
    
    auto* pnlPercentItem = new QTableWidgetItem("0.00%");
    pnlPercentItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionsTable_->setItem(newRow, 6, pnlPercentItem);
    
    // Calculate margin (simplified)
    double margin = entryPrice * size * 0.01; // 1% margin requirement
    auto* marginItem = new QTableWidgetItem(QString::number(margin, 'f', 2));
    marginItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    positionsTable_->setItem(newRow, 7, marginItem);
    
    positionsTable_->setItem(newRow, 8, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
    
    // Add close button
    auto* closeButton = new QPushButton("Close");
    closeButton->setMaximumHeight(25);
    closeButton->setStyleSheet("background-color: #ff4757; color: white; font-weight: bold;");
    connect(closeButton, &QPushButton::clicked, [this, newRow]() { 
        closePosition(newRow); 
    });
    positionsTable_->setCellWidget(newRow, 9, closeButton);
    
    openPositionsCount_++;
    totalExposure_ += margin;
    updateSummary();
    
    std::cout << "Position added: " << symbol.toStdString() 
              << " " << side.toStdString() 
              << " " << size << " @ " << entryPrice << std::endl;
}

void PositionWidget::clearPositions() {
    positionsTable_->setRowCount(0);
    openPositionsCount_ = 0;
    totalPnL_ = 0.0;
    totalExposure_ = 0.0;
    updateSummary();
}

} // namespace MasterMind 