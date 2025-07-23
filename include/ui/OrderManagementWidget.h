#ifndef MASTERMIND_ORDER_MANAGEMENT_WIDGET_H
#define MASTERMIND_ORDER_MANAGEMENT_WIDGET_H

#include <QtWidgets/QWidget>
#include <QtCore/QString>
#include <QtCore/QMap>

class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QTableWidget;
class QTimer;

namespace MasterMind {

/**
 * @brief Order data structure for order submission
 */
struct OrderData {
    QString symbol;
    QString side;
    QString type;
    double quantity = 0.0;
    double price = 0.0;
    double stopLoss = 0.0;
    double takeProfit = 0.0;
    QString timestamp;
};

/**
 * @brief Widget for order entry and management
 * 
 * Provides order entry forms, active order management, position sizing calculator,
 * and order history display as specified in the requirements.
 */
class OrderManagementWidget : public QWidget {
    Q_OBJECT

public:
    explicit OrderManagementWidget(QWidget* parent = nullptr);
    ~OrderManagementWidget();

signals:
    void orderSubmitted(const OrderData& order);
    void orderCancelled(const QString& orderId);
    void allOrdersCancelled();
    void ordersRefreshRequested();

private slots:
    void onSubmitOrder();
    void onBuyClicked();
    void onSellClicked();
    void onCancelAllOrders();
    void onOrderTypeChanged(const QString& orderType);
    void onSymbolChanged(const QString& symbol);
    void calculatePositionSize();
    void refreshActiveOrders();
    void cancelSelectedOrder();
    void updateOrderTables();

private:
    void setupUI();
    QWidget* createOrderEntrySection();
    QWidget* createActiveOrdersSection();
    QWidget* createOrderHistorySection();
    void setupActiveOrdersTable();
    void setupOrderHistoryTable();
    void connectSignals();
    void initializeData();
    void addSampleActiveOrders();
    void addSampleOrderHistory();
    void cancelOrder(int row);
    bool validateOrderForm();
    void resetOrderForm();
    double getCurrentMarketPrice(const QString& symbol);

    // UI components - Order Entry
    QComboBox* symbolCombo_;
    QComboBox* orderTypeCombo_;
    QComboBox* sideCombo_;
    QDoubleSpinBox* quantitySpinBox_;
    QDoubleSpinBox* priceSpinBox_;
    QDoubleSpinBox* stopLossSpinBox_;
    QDoubleSpinBox* takeProfitSpinBox_;
    QDoubleSpinBox* riskPercentSpinBox_;
    QPushButton* submitButton_;
    QPushButton* cancelAllButton_;
    QPushButton* positionSizeCalculator_;
    
    // UI components - Order Management
    QTableWidget* activeOrdersTable_;
    QTableWidget* orderHistoryTable_;
    QTimer* updateTimer_;
    
    // Data
    QString currentSide_;
};

} // namespace MasterMind

#endif // MASTERMIND_ORDER_MANAGEMENT_WIDGET_H 