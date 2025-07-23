#include "RenkoChartWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QDebug>
#include <algorithm>

namespace MasterMind {

RenkoChartWidget::RenkoChartWidget(QWidget* parent)
    : QWidget(parent)
    , brickSize_(0.001)
    , currentPrice_(0.0)
    , zoomLevel_(1.0)
    , panOffset_(0, 0)
    , isDragging_(false)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    
    // Set up update timer
    updateTimer_ = new QTimer(this);
    connect(updateTimer_, &QTimer::timeout, this, [this]() {
        update();
    });
    updateTimer_->start(100); // Update every 100ms
    
    // Initialize with some sample data
    initializeSampleData();
}

RenkoChartWidget::~RenkoChartWidget() = default;

void RenkoChartWidget::setBrickSize(double size) {
    if (size > 0) {
        brickSize_ = size;
        recalculateRenkoBricks();
        update();
    }
}

double RenkoChartWidget::getBrickSize() const {
    return brickSize_;
}

void RenkoChartWidget::addPriceUpdate(double price, const QDateTime& timestamp) {
    priceData_.append({price, timestamp});
    currentPrice_ = price;
    
    // Keep only last 10000 price points to manage memory
    if (priceData_.size() > 10000) {
        priceData_.removeFirst();
    }
    
    recalculateRenkoBricks();
}

void RenkoChartWidget::clearData() {
    priceData_.clear();
    renkoBricks_.clear();
    update();
}

void RenkoChartWidget::setZoom(double zoom) {
    zoomLevel_ = std::max(0.1, std::min(10.0, zoom));
    update();
}

void RenkoChartWidget::resetView() {
    zoomLevel_ = 1.0;
    panOffset_ = QPoint(0, 0);
    update();
}

void RenkoChartWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Clear background
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    if (renkoBricks_.isEmpty()) {
        drawNoDataMessage(painter);
        return;
    }
    
    // Calculate drawing parameters
    calculateDrawingParameters();
    
    // Draw grid
    drawGrid(painter);
    
    // Draw Renko bricks
    drawRenkoBricks(painter);
    
    // Draw current price line if available
    if (currentPrice_ > 0) {
        drawCurrentPriceLine(painter);
    }
    
    // Draw scale
    drawPriceScale(painter);
    
    // Draw timestamp scale
    drawTimeScale(painter);
}

void RenkoChartWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDragging_ = true;
        lastPanPoint_ = event->pos();
    }
}

void RenkoChartWidget::mouseMoveEvent(QMouseEvent* event) {
    if (isDragging_) {
        QPoint delta = event->pos() - lastPanPoint_;
        panOffset_ += delta;
        lastPanPoint_ = event->pos();
        update();
    }
}

void RenkoChartWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDragging_ = false;
    }
}

void RenkoChartWidget::wheelEvent(QWheelEvent* event) {
    double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        zoomLevel_ *= scaleFactor;
    } else {
        zoomLevel_ /= scaleFactor;
    }
    
    zoomLevel_ = std::max(0.1, std::min(10.0, zoomLevel_));
    update();
}

void RenkoChartWidget::initializeSampleData() {
    // Initialize with some sample price data for demonstration
    QDateTime baseTime = QDateTime::currentDateTime().addSecs(-3600); // 1 hour ago
    double basePrice = 1.1000;
    
    for (int i = 0; i < 1000; ++i) {
        double price = basePrice + (qrand() % 200 - 100) * 0.00001; // Random walk
        QDateTime time = baseTime.addSecs(i * 5); // Every 5 seconds
        addPriceUpdate(price, time);
        basePrice = price;
    }
}

void RenkoChartWidget::recalculateRenkoBricks() {
    if (priceData_.isEmpty() || brickSize_ <= 0) {
        return;
    }
    
    renkoBricks_.clear();
    
    double referencePrice = priceData_.first().price;
    double currentBrickLevel = qFloor(referencePrice / brickSize_) * brickSize_;
    
    for (const auto& pricePoint : priceData_) {
        double price = pricePoint.price;
        
        // Determine if we need to create new bricks
        while (price >= currentBrickLevel + brickSize_) {
            // Create up brick
            RenkoBrick brick;
            brick.open = currentBrickLevel;
            brick.close = currentBrickLevel + brickSize_;
            brick.high = brick.close;
            brick.low = brick.open;
            brick.timestamp = pricePoint.timestamp;
            brick.isUp = true;
            
            renkoBricks_.append(brick);
            currentBrickLevel += brickSize_;
        }
        
        while (price <= currentBrickLevel - brickSize_) {
            // Create down brick
            RenkoBrick brick;
            brick.open = currentBrickLevel;
            brick.close = currentBrickLevel - brickSize_;
            brick.high = brick.open;
            brick.low = brick.close;
            brick.timestamp = pricePoint.timestamp;
            brick.isUp = false;
            
            renkoBricks_.append(brick);
            currentBrickLevel -= brickSize_;
        }
    }
}

void RenkoChartWidget::calculateDrawingParameters() {
    if (renkoBricks_.isEmpty()) return;
    
    // Find price range
    minPrice_ = renkoBricks_.first().low;
    maxPrice_ = renkoBricks_.first().high;
    
    for (const auto& brick : renkoBricks_) {
        minPrice_ = std::min(minPrice_, brick.low);
        maxPrice_ = std::max(maxPrice_, brick.high);
    }
    
    double priceRange = maxPrice_ - minPrice_;
    if (priceRange == 0) priceRange = brickSize_;
    
    // Add some padding
    minPrice_ -= priceRange * 0.1;
    maxPrice_ += priceRange * 0.1;
    
    // Calculate brick dimensions
    int chartWidth = width() - 80; // Leave space for price scale
    int chartHeight = height() - 60; // Leave space for time scale
    
    brickWidth_ = std::max(5.0, (chartWidth * zoomLevel_) / std::max(1, renkoBricks_.size()));
    priceToPixel_ = chartHeight / (maxPrice_ - minPrice_);
}

void RenkoChartWidget::drawGrid(QPainter& painter) {
    painter.setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));
    
    int chartWidth = width() - 80;
    int chartHeight = height() - 60;
    
    // Horizontal grid lines (price levels)
    int numHorizontalLines = 10;
    for (int i = 0; i <= numHorizontalLines; ++i) {
        int y = 30 + (chartHeight * i) / numHorizontalLines;
        painter.drawLine(50, y, 50 + chartWidth, y);
    }
    
    // Vertical grid lines (time)
    int numVerticalLines = std::min(20, renkoBricks_.size());
    for (int i = 0; i <= numVerticalLines; ++i) {
        int x = 50 + (chartWidth * i) / numVerticalLines;
        painter.drawLine(x, 30, x, 30 + chartHeight);
    }
}

void RenkoChartWidget::drawRenkoBricks(QPainter& painter) {
    if (renkoBricks_.isEmpty()) return;
    
    int startIndex = std::max(0, static_cast<int>(-panOffset_.x() / brickWidth_));
    int endIndex = std::min(renkoBricks_.size(), startIndex + static_cast<int>(width() / brickWidth_) + 2);
    
    for (int i = startIndex; i < endIndex; ++i) {
        const auto& brick = renkoBricks_[i];
        
        // Calculate brick position and size
        double x = 50 + (i * brickWidth_) + panOffset_.x();
        double yHigh = 30 + (maxPrice_ - brick.high) * priceToPixel_;
        double yLow = 30 + (maxPrice_ - brick.low) * priceToPixel_;
        double height = yLow - yHigh;
        
        // Choose colors
        QColor fillColor = brick.isUp ? QColor(0, 200, 0, 150) : QColor(200, 0, 0, 150);
        QColor borderColor = brick.isUp ? QColor(0, 255, 0) : QColor(255, 0, 0);
        
        // Draw brick
        painter.fillRect(QRectF(x, yHigh, brickWidth_ - 1, height), fillColor);
        painter.setPen(QPen(borderColor, 1));
        painter.drawRect(QRectF(x, yHigh, brickWidth_ - 1, height));
    }
}

void RenkoChartWidget::drawCurrentPriceLine(QPainter& painter) {
    double y = 30 + (maxPrice_ - currentPrice_) * priceToPixel_;
    
    painter.setPen(QPen(QColor(255, 255, 0), 2, Qt::DashLine));
    painter.drawLine(50, y, width() - 30, y);
    
    // Draw price label
    painter.setPen(QPen(QColor(255, 255, 255)));
    painter.drawText(width() - 25, y + 5, QString::number(currentPrice_, 'f', 5));
}

void RenkoChartWidget::drawPriceScale(QPainter& painter) {
    painter.setPen(QPen(QColor(200, 200, 200)));
    
    int numLabels = 10;
    int chartHeight = height() - 60;
    
    for (int i = 0; i <= numLabels; ++i) {
        double price = minPrice_ + (maxPrice_ - minPrice_) * i / numLabels;
        int y = 30 + (chartHeight * (numLabels - i)) / numLabels;
        
        painter.drawText(5, y + 5, QString::number(price, 'f', 5));
        painter.drawLine(45, y, 50, y);
    }
}

void RenkoChartWidget::drawTimeScale(QPainter& painter) {
    if (renkoBricks_.isEmpty()) return;
    
    painter.setPen(QPen(QColor(200, 200, 200)));
    
    int numLabels = std::min(6, renkoBricks_.size());
    int chartWidth = width() - 80;
    
    for (int i = 0; i < numLabels; ++i) {
        int brickIndex = (renkoBricks_.size() * i) / (numLabels - 1);
        if (brickIndex >= renkoBricks_.size()) brickIndex = renkoBricks_.size() - 1;
        
        const auto& brick = renkoBricks_[brickIndex];
        int x = 50 + (chartWidth * i) / (numLabels - 1);
        
        QString timeStr = brick.timestamp.toString("hh:mm");
        painter.drawText(x - 20, height() - 5, timeStr);
        painter.drawLine(x, height() - 30, x, height() - 35);
    }
}

void RenkoChartWidget::drawNoDataMessage(QPainter& painter) {
    painter.setPen(QPen(QColor(150, 150, 150)));
    painter.drawText(rect(), Qt::AlignCenter, "No price data available");
}

} // namespace MasterMind 