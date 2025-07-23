#include "ui/OrderManagementWidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtGui/QFont>
#include <iostream>

namespace MasterMind {

OrderManagementWidget::OrderManagementWidget(QWidget* parent)
    : QWidget(parent)
    , symbolCombo_(nullptr)
    , orderTypeCombo_(nullptr)
    , sideCombo_(nullptr)
    , quantitySpinBox_(nullptr)
    , priceSpinBox_(nullptr)
    , stopLossSpinBox_(nullptr)
    , takeProfitSpinBox_(nullptr)
    , submitButton_(nullptr)
    , cancelAllButton_(nullptr)
    , activeOrdersTable_(nullptr)
    , orderHistoryTable_(nullptr)
    , updateTimer_(nullptr)
    , positionSizeCalculator_(nullptr)
    , riskPercentSpinBox_(nullptr)
{
    setupUI();
    connectSignals();
    initializeData();
    
    std::cout << "OrderManagementWidget initialized" << std::endl;
}

OrderManagementWidget::~OrderManagementWidget() = default;

void OrderManagementWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    
    // Title
    auto* titleLabel = new QLabel("Order Management");
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Create order entry section
    layout->addWidget(createOrderEntrySection());
    
    // Create active orders section
    layout->addWidget(createActiveOrdersSection());
    
    // Create order history section
    layout->addWidget(createOrderHistorySection());
    
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
        QPushButton#buyButton {
            background-color: #00ff88;
            color: #000000;
        }
        QPushButton#sellButton {
            background-color: #ff4757;
            color: #ffffff;
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
    )");
}

QWidget* OrderManagementWidget::createOrderEntrySection() {
    auto* groupBox = new QGroupBox("Order Entry");
    auto* layout = new QGridLayout(groupBox);
    
    int row = 0;
    
    // Symbol selection
    layout->addWidget(new QLabel("Symbol:"), row, 0);
    symbolCombo_ = new QComboBox();
    symbolCombo_->addItems({"EURUSD", "GBPUSD", "USDJPY", "BTCUSD", "ETHUSD", "XAUUSD"});
    symbolCombo_->setMinimumWidth(100);
    layout->addWidget(symbolCombo_, row, 1);
    
    // Order type
    layout->addWidget(new QLabel("Type:"), row, 2);
    orderTypeCombo_ = new QComboBox();
    orderTypeCombo_->addItems({"Market", "Limit", "Stop", "Stop Limit"});
    layout->addWidget(orderTypeCombo_, row, 3);
    
    row++;
    
    // Quantity
    layout->addWidget(new QLabel("Quantity:"), row, 0);
    quantitySpinBox_ = new QDoubleSpinBox();
    quantitySpinBox_->setRange(0.01, 1000.0);
    quantitySpinBox_->setSingleStep(0.01);
    quantitySpinBox_->setDecimals(2);
    quantitySpinBox_->setValue(0.10);
    layout->addWidget(quantitySpinBox_, row, 1);
    
    // Price
    layout->addWidget(new QLabel("Price:"), row, 2);
    priceSpinBox_ = new QDoubleSpinBox();
    priceSpinBox_->setRange(0.00001, 100000.0);
    priceSpinBox_->setSingleStep(0.0001);
    priceSpinBox_->setDecimals(5);
    priceSpinBox_->setValue(1.10000);
    layout->addWidget(priceSpinBox_, row, 3);
    
    row++;
    
    // Stop Loss
    layout->addWidget(new QLabel("Stop Loss:"), row, 0);
    stopLossSpinBox_ = new QDoubleSpinBox();
    stopLossSpinBox_->setRange(0.0, 100000.0);
    stopLossSpinBox_->setSingleStep(0.0001);
    stopLossSpinBox_->setDecimals(5);
    stopLossSpinBox_->setValue(0.0);
    layout->addWidget(stopLossSpinBox_, row, 1);
    
    // Take Profit
    layout->addWidget(new QLabel("Take Profit:"), row, 2);
    takeProfitSpinBox_ = new QDoubleSpinBox();
    takeProfitSpinBox_->setRange(0.0, 100000.0);
    takeProfitSpinBox_->setSingleStep(0.0001);
    takeProfitSpinBox_->setDecimals(5);
    takeProfitSpinBox_->setValue(0.0);
    layout->addWidget(takeProfitSpinBox_, row, 3);
    
    row++;
    
    // Risk management section
    layout->addWidget(new QLabel("Risk %:"), row, 0);
    riskPercentSpinBox_ = new QDoubleSpinBox();
    riskPercentSpinBox_->setRange(0.1, 10.0);
    riskPercentSpinBox_->setSingleStep(0.1);
    riskPercentSpinBox_->setDecimals(1);
    riskPercentSpinBox_->setValue(2.0);
    riskPercentSpinBox_->setSuffix("%");
    layout->addWidget(riskPercentSpinBox_, row, 1);
    
    // Position size calculator
    positionSizeCalculator_ = new QPushButton("Calculate Size");
    layout->addWidget(positionSizeCalculator_, row, 2);
    
    row++;
    
    // Order buttons
    auto* buttonLayout = new QHBoxLayout();
    
    auto* buyButton = new QPushButton("BUY");
    buyButton->setObjectName("buyButton");
    buyButton->setMinimumHeight(40);
    buttonLayout->addWidget(buyButton);
    
    auto* sellButton = new QPushButton("SELL");
    sellButton->setObjectName("sellButton");
    sellButton->setMinimumHeight(40);
    buttonLayout->addWidget(sellButton);
    
    submitButton_ = new QPushButton("Submit Order");
    submitButton_->setMinimumHeight(40);
    buttonLayout->addWidget(submitButton_);
    
    cancelAllButton_ = new QPushButton("Cancel All");
    cancelAllButton_->setMinimumHeight(40);
    buttonLayout->addWidget(cancelAllButton_);
    
    layout->addLayout(buttonLayout, row, 0, 1, 4);
    
    // Connect buttons
    connect(buyButton, &QPushButton::clicked, [this]() { 
        sideCombo_ = new QComboBox(); // Temporary for logic
        onBuyClicked(); 
    });
    connect(sellButton, &QPushButton::clicked, [this]() { 
        sideCombo_ = new QComboBox(); // Temporary for logic
        onSellClicked(); 
    });
    
    return groupBox;
}

QWidget* OrderManagementWidget::createActiveOrdersSection() {
    auto* groupBox = new QGroupBox("Active Orders");
    auto* layout = new QVBoxLayout(groupBox);
    
    // Active orders table
    activeOrdersTable_ = new QTableWidget();
    setupActiveOrdersTable();
    layout->addWidget(activeOrdersTable_);
    
    // Control buttons
    auto* buttonLayout = new QHBoxLayout();
    
    auto* refreshButton = new QPushButton("Refresh");
    connect(refreshButton, &QPushButton::clicked, this, &OrderManagementWidget::refreshActiveOrders);
    buttonLayout->addWidget(refreshButton);
    
    auto* cancelSelectedButton = new QPushButton("Cancel Selected");
    connect(cancelSelectedButton, &QPushButton::clicked, this, &OrderManagementWidget::cancelSelectedOrder);
    buttonLayout->addWidget(cancelSelectedButton);
    
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
    
    return groupBox;
}

QWidget* OrderManagementWidget::createOrderHistorySection() {
    auto* groupBox = new QGroupBox("Order History");
    auto* layout = new QVBoxLayout(groupBox);
    
    // Order history table
    orderHistoryTable_ = new QTableWidget();
    setupOrderHistoryTable();
    layout->addWidget(orderHistoryTable_);
    
    return groupBox;
}

void OrderManagementWidget::setupActiveOrdersTable() {
    QStringList headers = {"Order ID", "Symbol", "Side", "Type", "Quantity", "Price", "Status", "Time", "Actions"};
    activeOrdersTable_->setColumnCount(headers.size());
    activeOrdersTable_->setHorizontalHeaderLabels(headers);
    
    activeOrdersTable_->setAlternatingRowColors(true);
    activeOrdersTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    activeOrdersTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Set column widths
    activeOrdersTable_->setColumnWidth(0, 120); // Order ID
    activeOrdersTable_->setColumnWidth(1, 80);  // Symbol
    activeOrdersTable_->setColumnWidth(2, 60);  // Side
    activeOrdersTable_->setColumnWidth(3, 80);  // Type
    activeOrdersTable_->setColumnWidth(4, 80);  // Quantity
    activeOrdersTable_->setColumnWidth(5, 80);  // Price
    activeOrdersTable_->setColumnWidth(6, 80);  // Status
    activeOrdersTable_->setColumnWidth(7, 120); // Time
    
    activeOrdersTable_->horizontalHeader()->setStretchLastSection(true);
    activeOrdersTable_->verticalHeader()->setVisible(false);
}

void OrderManagementWidget::setupOrderHistoryTable() {
    QStringList headers = {"Order ID", "Symbol", "Side", "Type", "Quantity", "Price", "Status", "Fill Price", "PnL", "Time"};
    orderHistoryTable_->setColumnCount(headers.size());
    orderHistoryTable_->setHorizontalHeaderLabels(headers);
    
    orderHistoryTable_->setAlternatingRowColors(true);
    orderHistoryTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderHistoryTable_->setMaximumHeight(200);
    
    // Set column widths
    for (int i = 0; i < headers.size(); ++i) {
        orderHistoryTable_->setColumnWidth(i, 80);
    }
    
    orderHistoryTable_->horizontalHeader()->setStretchLastSection(true);
    orderHistoryTable_->verticalHeader()->setVisible(false);
}

void OrderManagementWidget::connectSignals() {
    // Setup update timer
    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(2000); // 2 second updates
    connect(updateTimer_, &QTimer::timeout, this, &OrderManagementWidget::updateOrderTables);
    updateTimer_->start();
    
    // Connect form elements
    connect(submitButton_, &QPushButton::clicked, this, &OrderManagementWidget::onSubmitOrder);
    connect(cancelAllButton_, &QPushButton::clicked, this, &OrderManagementWidget::onCancelAllOrders);
    connect(positionSizeCalculator_, &QPushButton::clicked, this, &OrderManagementWidget::calculatePositionSize);
    
    // Connect order type changes to enable/disable price field
    connect(orderTypeCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &OrderManagementWidget::onOrderTypeChanged);
    
    // Connect symbol changes to update price
    connect(symbolCombo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &OrderManagementWidget::onSymbolChanged);
}

void OrderManagementWidget::initializeData() {
    // Add some sample active orders
    addSampleActiveOrders();
    
    // Add some sample order history
    addSampleOrderHistory();
    
    // Set initial form state
    onOrderTypeChanged(orderTypeCombo_->currentText());
    onSymbolChanged(symbolCombo_->currentText());
}

void OrderManagementWidget::addSampleActiveOrders() {
    // Sample active orders for demonstration
    QStringList sampleOrders = {
        "MM1234567890|EURUSD|BUY|LIMIT|0.10|1.1050|PENDING|10:30:45",
        "MM1234567891|GBPUSD|SELL|STOP|0.20|1.2800|PENDING|10:32:12",
        "MM1234567892|BTCUSD|BUY|LIMIT|0.01|45000.0|PENDING|10:35:22"
    };
    
    activeOrdersTable_->setRowCount(sampleOrders.size());
    
    for (int i = 0; i < sampleOrders.size(); ++i) {
        QStringList orderData = sampleOrders[i].split("|");
        for (int j = 0; j < orderData.size(); ++j) {
            auto* item = new QTableWidgetItem(orderData[j]);
            if (j == 4 || j == 5) { // Quantity and Price columns
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            }
            activeOrdersTable_->setItem(i, j, item);
        }
        
        // Add cancel button in actions column
        auto* cancelButton = new QPushButton("Cancel");
        cancelButton->setMaximumHeight(25);
        connect(cancelButton, &QPushButton::clicked, [this, i]() { 
            cancelOrder(i); 
        });
        activeOrdersTable_->setCellWidget(i, 8, cancelButton);
    }
}

void OrderManagementWidget::addSampleOrderHistory() {
    // Sample order history for demonstration
    QStringList sampleHistory = {
        "MM1234567880|EURUSD|BUY|MARKET|0.10|1.1000|FILLED|1.1001|+1.0|09:45:12",
        "MM1234567881|GBPUSD|SELL|LIMIT|0.20|1.2850|FILLED|1.2849|-2.0|09:52:34",
        "MM1234567882|USDJPY|BUY|MARKET|0.15|110.50|FILLED|110.52|+3.0|10:15:55"
    };
    
    orderHistoryTable_->setRowCount(sampleHistory.size());
    
    for (int i = 0; i < sampleHistory.size(); ++i) {
        QStringList orderData = sampleHistory[i].split("|");
        for (int j = 0; j < orderData.size(); ++j) {
            auto* item = new QTableWidgetItem(orderData[j]);
            if (j == 4 || j == 5 || j == 7) { // Quantity, Price, Fill Price columns
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            }
            
            // Color code PnL
            if (j == 8) { // PnL column
                if (orderData[j].startsWith("+")) {
                    item->setForeground(QColor(0, 255, 136)); // Green for profit
                } else if (orderData[j].startsWith("-")) {
                    item->setForeground(QColor(255, 71, 87)); // Red for loss
                }
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            }
            
            orderHistoryTable_->setItem(i, j, item);
        }
    }
}

void OrderManagementWidget::onSubmitOrder() {
    // Validate form data
    if (!validateOrderForm()) {
        return;
    }
    
    // Create order data
    OrderData order;
    order.symbol = symbolCombo_->currentText();
    order.type = orderTypeCombo_->currentText();
    order.quantity = quantitySpinBox_->value();
    order.price = priceSpinBox_->value();
    order.stopLoss = stopLossSpinBox_->value();
    order.takeProfit = takeProfitSpinBox_->value();
    order.timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    
    // Emit signal to trading engine
    emit orderSubmitted(order);
    
    // Show confirmation
    QMessageBox::information(this, "Order Submitted", 
                           QString("Order submitted for %1 %2 %3 @ %4")
                           .arg(order.quantity)
                           .arg(order.symbol)
                           .arg(order.side)
                           .arg(order.price, 0, 'f', 5));
    
    // Clear form
    resetOrderForm();
    
    std::cout << "Order submitted: " << order.symbol.toStdString() 
              << " " << order.quantity << " @ " << order.price << std::endl;
}

void OrderManagementWidget::onBuyClicked() {
    currentSide_ = "BUY";
    onSubmitOrder();
}

void OrderManagementWidget::onSellClicked() {
    currentSide_ = "SELL";
    onSubmitOrder();
}

void OrderManagementWidget::onCancelAllOrders() {
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Cancel All Orders",
        "Are you sure you want to cancel all active orders?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Cancel all orders
        activeOrdersTable_->setRowCount(0);
        emit allOrdersCancelled();
        
        QMessageBox::information(this, "Orders Canceled", "All active orders have been canceled.");
    }
}

void OrderManagementWidget::onOrderTypeChanged(const QString& orderType) {
    // Enable/disable price field based on order type
    bool priceEnabled = (orderType == "Limit" || orderType == "Stop Limit");
    priceSpinBox_->setEnabled(priceEnabled);
    
    if (!priceEnabled && orderType == "Market") {
        // For market orders, use current market price (simulated)
        priceSpinBox_->setValue(getCurrentMarketPrice(symbolCombo_->currentText()));
    }
}

void OrderManagementWidget::onSymbolChanged(const QString& symbol) {
    // Update price field with current market price for the symbol
    double marketPrice = getCurrentMarketPrice(symbol);
    priceSpinBox_->setValue(marketPrice);
    
    // Update stop loss and take profit suggestions
    if (symbol.contains("USD") && !symbol.startsWith("USD")) {
        // For XXXUSD pairs
        stopLossSpinBox_->setValue(marketPrice - 0.0020); // 20 pips below
        takeProfitSpinBox_->setValue(marketPrice + 0.0040); // 40 pips above
    } else if (symbol.contains("JPY")) {
        // For XXXJPY pairs
        stopLossSpinBox_->setValue(marketPrice - 0.20); // 20 pips below
        takeProfitSpinBox_->setValue(marketPrice + 0.40); // 40 pips above
    } else {
        // For crypto and other pairs
        double spread = marketPrice * 0.002; // 0.2%
        stopLossSpinBox_->setValue(marketPrice - spread * 2);
        takeProfitSpinBox_->setValue(marketPrice + spread * 4);
    }
}

void OrderManagementWidget::calculatePositionSize() {
    QString symbol = symbolCombo_->currentText();
    double riskPercent = riskPercentSpinBox_->value() / 100.0;
    double stopLoss = stopLossSpinBox_->value();
    double entryPrice = priceSpinBox_->value();
    
    if (stopLoss <= 0 || entryPrice <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Please set valid entry price and stop loss.");
        return;
    }
    
    // Simple position size calculation (assuming $10,000 account)
    double accountBalance = 10000.0;
    double riskAmount = accountBalance * riskPercent;
    double stopDistance = std::abs(entryPrice - stopLoss);
    
    if (stopDistance <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Stop loss must be different from entry price.");
        return;
    }
    
    double positionSize = riskAmount / stopDistance;
    
    // Apply minimum and maximum limits
    positionSize = std::max(0.01, std::min(positionSize, 10.0));
    
    quantitySpinBox_->setValue(positionSize);
    
    QMessageBox::information(this, "Position Size Calculated",
                           QString("Recommended position size: %1\n"
                                  "Risk amount: $%2\n"
                                  "Stop distance: %3")
                           .arg(positionSize, 0, 'f', 2)
                           .arg(riskAmount, 0, 'f', 2)
                           .arg(stopDistance, 0, 'f', 5));
}

void OrderManagementWidget::refreshActiveOrders() {
    // Simulate refreshing orders
    emit ordersRefreshRequested();
    std::cout << "Refreshing active orders..." << std::endl;
}

void OrderManagementWidget::cancelSelectedOrder() {
    int currentRow = activeOrdersTable_->currentRow();
    if (currentRow >= 0) {
        QString orderId = activeOrdersTable_->item(currentRow, 0)->text();
        
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Cancel Order",
            QString("Cancel order %1?").arg(orderId),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            cancelOrder(currentRow);
        }
    } else {
        QMessageBox::information(this, "No Selection", "Please select an order to cancel.");
    }
}

void OrderManagementWidget::cancelOrder(int row) {
    if (row >= 0 && row < activeOrdersTable_->rowCount()) {
        QString orderId = activeOrdersTable_->item(row, 0)->text();
        
        // Remove from active orders table
        activeOrdersTable_->removeRow(row);
        
        emit orderCancelled(orderId);
        
        std::cout << "Order cancelled: " << orderId.toStdString() << std::endl;
    }
}

void OrderManagementWidget::updateOrderTables() {
    // Simulate order status updates
    for (int i = 0; i < activeOrdersTable_->rowCount(); ++i) {
        // Randomly update some orders to "FILLED" status
        if (qrand() % 100 < 5) { // 5% chance
            auto* statusItem = activeOrdersTable_->item(i, 6);
            if (statusItem && statusItem->text() == "PENDING") {
                statusItem->setText("FILLED");
                statusItem->setForeground(QColor(0, 255, 136)); // Green
            }
        }
    }
}

bool OrderManagementWidget::validateOrderForm() {
    if (symbolCombo_->currentText().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please select a symbol.");
        return false;
    }
    
    if (quantitySpinBox_->value() <= 0) {
        QMessageBox::warning(this, "Validation Error", "Quantity must be greater than 0.");
        return false;
    }
    
    if (orderTypeCombo_->currentText() != "Market" && priceSpinBox_->value() <= 0) {
        QMessageBox::warning(this, "Validation Error", "Price must be greater than 0 for non-market orders.");
        return false;
    }
    
    return true;
}

void OrderManagementWidget::resetOrderForm() {
    quantitySpinBox_->setValue(0.10);
    stopLossSpinBox_->setValue(0.0);
    takeProfitSpinBox_->setValue(0.0);
    riskPercentSpinBox_->setValue(2.0);
}

double OrderManagementWidget::getCurrentMarketPrice(const QString& symbol) {
    // Simulate getting current market price
    static QMap<QString, double> prices = {
        {"EURUSD", 1.1000},
        {"GBPUSD", 1.2800},
        {"USDJPY", 110.50},
        {"BTCUSD", 45000.0},
        {"ETHUSD", 3000.0},
        {"XAUUSD", 1950.0}
    };
    
    return prices.value(symbol, 1.0);
}

} // namespace MasterMind 