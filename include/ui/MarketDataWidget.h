#ifndef MARKET_DATA_WIDGET_H
#define MARKET_DATA_WIDGET_H

#include <QtWidgets/QWidget>

class MarketDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MarketDataWidget(QWidget *parent = nullptr);
    ~MarketDataWidget();

signals:
    void priceUpdated(const QString& symbol, double price);

public slots:
    void updateMarketData();

private:
    // Market data display implementation
};

#endif // MARKET_DATA_WIDGET_H 