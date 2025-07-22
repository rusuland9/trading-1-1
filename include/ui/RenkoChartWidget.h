#ifndef RENKO_CHART_WIDGET_H
#define RENKO_CHART_WIDGET_H

#include <QtWidgets/QWidget>

class RenkoChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RenkoChartWidget(QWidget *parent = nullptr);
    ~RenkoChartWidget();

signals:
    void patternDetected(const QString& pattern);

public slots:
    void updatePrice(double price);
    void updateBrickSize(double size);

private:
    // Will be implemented with Qt Charts
};

#endif // RENKO_CHART_WIDGET_H 