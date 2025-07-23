#include "ui/MarketDataWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QFont>
#include <iostream>

namespace MasterMind {

MarketDataWidget::MarketDataWidget(QWidget* parent)
    : QWidget(parent)
    , symbolSelector_(nullptr)
    , refreshButton_(nullptr)
    , dataTable_(nullptr)
    , updateTimer_(nullptr)
    , statusLabel_(nullptr)
{
    setupUI();
    connectSignals();
    
    // Initialize with sample data
    initializeSampleData();
    
    std::cout << "MarketDataWidget initialized" << std::endl;
}

MarketDataWidget::~MarketDataWidget() = default;

void MarketDataWidget::addSymbol(const QString& symbol) {
    if (!symbols_.contains(symbol)) {
        symbols_.append(symbol);
        symbolSelector_->addItem(symbol);
        
        // Initialize market data for the symbol
        MarketData data;
        data.symbol = symbol;
        data.bid = 0.0;
        data.ask = 0.0;
        data.last = 0.0;
        data.volume = 0.0;
        data.change = 0.0;
        data.changePercent = 0.0;
        data.timestamp = QDateTime::currentDateTime();
        
        marketData_[symbol] = data;
        updateTable();
    }
}

void MarketDataWidget::removeSymbol(const QString& symbol) {
    int index = symbols_.indexOf(symbol);
    if (index >= 0) {
        symbols_.removeAt(index);
        symbolSelector_->removeItem(index);
        marketData_.remove(symbol);
        updateTable();
    }
}

void MarketDataWidget::updateMarketData(const QString& symbol, const MarketData& data) {
    if (marketData_.contains(symbol)) {
        marketData_[symbol] = data;
        updateTable();
    }
}

void MarketDataWidget::updateTickData(const QString& symbol, double bid, double ask, double last, double volume) {
    if (marketData_.contains(symbol)) {
        auto& data = marketData_[symbol];
        
        // Calculate change
        double previousLast = data.last;
        data.bid = bid;
        data.ask = ask;
        data.last = last;
        data.volume = volume;
        data.timestamp = QDateTime::currentDateTime();
        
        if (previousLast > 0) {
            data.change = last - previousLast;
            data.changePercent = (data.change / previousLast) * 100.0;
        }
        
        updateTable();
    }
}

void MarketDataWidget::clearData() {
    marketData_.clear();
    updateTable();
}

void MarketDataWidget::setUpdateInterval(int milliseconds) {
    if (updateTimer_) {
        updateTimer_->setInterval(milliseconds);
    }
}

void MarketDataWidget::startUpdates() {
    if (updateTimer_) {
        updateTimer_->start();
        statusLabel_->setText("Status: Updating");
    }
}

void MarketDataWidget::stopUpdates() {
    if (updateTimer_) {
        updateTimer_->stop();
        statusLabel_->setText("Status: Stopped");
    }
}

void MarketDataWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Header section
    auto* headerLayout = new QHBoxLayout();
    
    // Title
    auto* titleLabel = new QLabel("Market Data");
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    // Symbol selector
    symbolSelector_ = new QComboBox();
    symbolSelector_->setMinimumWidth(120);
    symbolSelector_->setToolTip("Select symbol to view details");
    headerLayout->addWidget(new QLabel("Symbol:"));
    headerLayout->addWidget(symbolSelector_);
    
    // Refresh button
    refreshButton_ = new QPushButton("Refresh");
    refreshButton_->setToolTip("Refresh market data");
    headerLayout->addWidget(refreshButton_);
    
    layout->addLayout(headerLayout);
    
    // Market data table
    dataTable_ = new QTableWidget();
    setupTable();
    layout->addWidget(dataTable_);
    
    // Status section
    auto* statusLayout = new QHBoxLayout();
    statusLabel_ = new QLabel("Status: Ready");
    statusLayout->addWidget(statusLabel_);
    statusLayout->addStretch();
    
    auto* lastUpdateLabel = new QLabel("Last Update:");
    lastUpdateTime_ = new QLabel("--:--:--");
    statusLayout->addWidget(lastUpdateLabel);
    statusLayout->addWidget(lastUpdateTime_);
    
    layout->addLayout(statusLayout);
    
    // Set styling
    setStyleSheet(R"(
        QTableWidget {
            alternate-background-color: #2d2d3a;
            background-color: #1e1e2e;
            gridline-color: #4d4d5a;
            color: #ffffff;
        }
        QTableWidget::item {
            padding: 4px;
        }
        QHeaderView::section {
            background-color: #3d3d4a;
            color: #ffffff;
            border: 1px solid #4d4d5a;
            font-weight: bold;
        }
    )");
}

void MarketDataWidget::setupTable() {
    // Set up table columns
    QStringList headers = {"Symbol", "Bid", "Ask", "Last", "Change", "Change %", "Volume", "Time"};
    dataTable_->setColumnCount(headers.size());
    dataTable_->setHorizontalHeaderLabels(headers);
    
    // Configure table appearance
    dataTable_->setAlternatingRowColors(true);
    dataTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    dataTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    dataTable_->setSortingEnabled(true);
    
    // Set column widths
    dataTable_->horizontalHeader()->setStretchLastSection(true);
    dataTable_->setColumnWidth(0, 80);  // Symbol
    dataTable_->setColumnWidth(1, 80);  // Bid
    dataTable_->setColumnWidth(2, 80);  // Ask
    dataTable_->setColumnWidth(3, 80);  // Last
    dataTable_->setColumnWidth(4, 80);  // Change
    dataTable_->setColumnWidth(5, 80);  // Change %
    dataTable_->setColumnWidth(6, 100); // Volume
    
    // Hide row numbers
    dataTable_->verticalHeader()->setVisible(false);
}

void MarketDataWidget::connectSignals() {
    // Setup update timer
    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(1000); // 1 second updates
    connect(updateTimer_, &QTimer::timeout, this, &MarketDataWidget::onUpdateTimer);
    
    // Connect refresh button
    connect(refreshButton_, &QPushButton::clicked, this, &MarketDataWidget::onRefreshClicked);
    
    // Connect symbol selector
    connect(symbolSelector_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &MarketDataWidget::onSymbolSelected);
}

void MarketDataWidget::updateTable() {
    dataTable_->setRowCount(marketData_.size());
    
    int row = 0;
    for (auto it = marketData_.constBegin(); it != marketData_.constEnd(); ++it, ++row) {
        const MarketData& data = it.value();
        
        // Symbol
        dataTable_->setItem(row, 0, new QTableWidgetItem(data.symbol));
        
        // Bid
        auto* bidItem = new QTableWidgetItem(QString::number(data.bid, 'f', 5));
        bidItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dataTable_->setItem(row, 1, bidItem);
        
        // Ask
        auto* askItem = new QTableWidgetItem(QString::number(data.ask, 'f', 5));
        askItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dataTable_->setItem(row, 2, askItem);
        
        // Last
        auto* lastItem = new QTableWidgetItem(QString::number(data.last, 'f', 5));
        lastItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dataTable_->setItem(row, 3, lastItem);
        
        // Change
        auto* changeItem = new QTableWidgetItem(QString::number(data.change, 'f', 5));
        changeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (data.change > 0) {
            changeItem->setForeground(QColor(0, 255, 136)); // Green
        } else if (data.change < 0) {
            changeItem->setForeground(QColor(255, 71, 87)); // Red
        }
        dataTable_->setItem(row, 4, changeItem);
        
        // Change %
        auto* changePercentItem = new QTableWidgetItem(QString::number(data.changePercent, 'f', 2) + "%");
        changePercentItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (data.changePercent > 0) {
            changePercentItem->setForeground(QColor(0, 255, 136)); // Green
        } else if (data.changePercent < 0) {
            changePercentItem->setForeground(QColor(255, 71, 87)); // Red
        }
        dataTable_->setItem(row, 5, changePercentItem);
        
        // Volume
        auto* volumeItem = new QTableWidgetItem(QString::number(data.volume, 'f', 0));
        volumeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        dataTable_->setItem(row, 6, volumeItem);
        
        // Time
        dataTable_->setItem(row, 7, new QTableWidgetItem(data.timestamp.toString("hh:mm:ss")));
    }
    
    // Update last update time
    lastUpdateTime_->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
}

void MarketDataWidget::initializeSampleData() {
    // Add some sample symbols and data
    QStringList sampleSymbols = {"EURUSD", "GBPUSD", "USDJPY", "BTCUSD", "ETHUSD"};
    
    for (const QString& symbol : sampleSymbols) {
        addSymbol(symbol);
        
        // Generate sample market data
        double basePrice = (symbol.contains("USD") && !symbol.startsWith("USD")) ? 1.1000 : 
                          (symbol.contains("BTC") ? 45000.0 : 
                          (symbol.contains("ETH") ? 3000.0 : 110.0));
        
        MarketData data;
        data.symbol = symbol;
        data.bid = basePrice - 0.0001;
        data.ask = basePrice + 0.0001;
        data.last = basePrice;
        data.volume = 1000000 + (qrand() % 500000);
        data.change = (qrand() % 200 - 100) * 0.0001;
        data.changePercent = (data.change / basePrice) * 100.0;
        data.timestamp = QDateTime::currentDateTime();
        
        marketData_[symbol] = data;
    }
    
    updateTable();
    startUpdates();
}

void MarketDataWidget::onUpdateTimer() {
    // Simulate real-time price updates
    for (auto it = marketData_.begin(); it != marketData_.end(); ++it) {
        MarketData& data = it.value();
        
        // Simulate price movement
        double change = (qrand() % 20 - 10) * 0.00001; // Small random changes
        data.last += change;
        data.bid = data.last - 0.0001;
        data.ask = data.last + 0.0001;
        data.change += change;
        data.changePercent = (data.change / (data.last - data.change)) * 100.0;
        data.timestamp = QDateTime::currentDateTime();
        
        // Update volume occasionally
        if (qrand() % 10 == 0) {
            data.volume += qrand() % 10000;
        }
    }
    
    updateTable();
    emit marketDataUpdated();
}

void MarketDataWidget::onRefreshClicked() {
    // Simulate refresh by generating new sample data
    onUpdateTimer();
    statusLabel_->setText("Status: Refreshed");
    
    // Reset status after 2 seconds
    QTimer::singleShot(2000, [this]() {
        statusLabel_->setText("Status: Updating");
    });
}

void MarketDataWidget::onSymbolSelected(const QString& symbol) {
    if (!symbol.isEmpty() && marketData_.contains(symbol)) {
        emit symbolSelected(symbol);
    }
}

} // namespace MasterMind 