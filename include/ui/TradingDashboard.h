#ifndef TRADING_DASHBOARD_H
#define TRADING_DASHBOARD_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

class TradingDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit TradingDashboard(QWidget *parent = nullptr);
    ~TradingDashboard();

private:
    QVBoxLayout *mainLayout_;
    QLabel *titleLabel_;
};

#endif // TRADING_DASHBOARD_H 