#pragma once

#include <QWidget>
#include <QDateTime>
#include <QTimer>
#include <QVector>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace MasterMind {

struct PriceData {
    double price;
    QDateTime timestamp;
};

struct RenkoBrick {
    double open;
    double close;
    double high;
    double low;
    QDateTime timestamp;
    bool isUp;
};

class RenkoChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit RenkoChartWidget(QWidget* parent = nullptr);
    ~RenkoChartWidget();

    // Configuration
    void setBrickSize(double size);
    double getBrickSize() const;
    
    // Data management
    void addPriceUpdate(double price, const QDateTime& timestamp = QDateTime::currentDateTime());
    void clearData();
    
    // View controls
    void setZoom(double zoom);
    void resetView();

public slots:
    void onPriceUpdate(double price) {
        addPriceUpdate(price);
    }

signals:
    void brickCreated(const RenkoBrick& brick);
    void patternDetected(const QString& pattern);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void initializeSampleData();
    void recalculateRenkoBricks();
    void calculateDrawingParameters();
    
    // Drawing methods
    void drawGrid(QPainter& painter);
    void drawRenkoBricks(QPainter& painter);
    void drawCurrentPriceLine(QPainter& painter);
    void drawPriceScale(QPainter& painter);
    void drawTimeScale(QPainter& painter);
    void drawNoDataMessage(QPainter& painter);

    // Data
    QVector<PriceData> priceData_;
    QVector<RenkoBrick> renkoBricks_;
    double brickSize_;
    double currentPrice_;
    
    // Drawing parameters
    double minPrice_;
    double maxPrice_;
    double brickWidth_;
    double priceToPixel_;
    
    // View state
    double zoomLevel_;
    QPoint panOffset_;
    bool isDragging_;
    QPoint lastPanPoint_;
    
    // UI
    QTimer* updateTimer_;
};

} // namespace MasterMind 