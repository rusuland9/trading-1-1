#ifndef RISK_MANAGEMENT_WIDGET_H
#define RISK_MANAGEMENT_WIDGET_H

#include <QtWidgets/QWidget>

class RiskManagementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RiskManagementWidget(QWidget *parent = nullptr);
    ~RiskManagementWidget();

public slots:
    void updateRiskMetrics();

private:
    // Risk management implementation
};

#endif // RISK_MANAGEMENT_WIDGET_H 