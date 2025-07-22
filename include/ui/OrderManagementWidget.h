#ifndef ORDER_MANAGEMENT_WIDGET_H
#define ORDER_MANAGEMENT_WIDGET_H

#include <QtWidgets/QWidget>

class OrderManagementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderManagementWidget(QWidget *parent = nullptr);
    ~OrderManagementWidget();

signals:
    void orderPlaced(const QString& symbol, double price, double quantity);

public slots:
    void updateOrders();

private:
    // Order management implementation
};

#endif // ORDER_MANAGEMENT_WIDGET_H 