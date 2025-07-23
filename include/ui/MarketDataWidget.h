#ifndef MASTERMIND_MARKET_DATA_WIDGET_H
#define MASTERMIND_MARKET_DATA_WIDGET_H

#include <QtWidgets/QWidget>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QDateTime>

class QComboBox;
class QPushButton;
class QTableWidget;
class QTimer;
class QLabel;

namespace MasterMind {

/**
 * @brief Market data structure for displaying price information
 */
struct MarketData {
    QString symbol;
    double bid = 0.0;
    double ask = 0.0;
    double last = 0.0;
    double volume = 0.0;
    double change = 0.0;
    double changePercent = 0.0;
    QDateTime timestamp;
};

/**
 * @brief Widget for displaying real-time market data
 * 
 * Shows live price feeds with bid/ask spreads, last prices, volume indicators,
 * and price changes for multiple symbols in a table format.
 */
class MarketDataWidget : public QWidget {
    Q_OBJECT

public:
    explicit MarketDataWidget(QWidget* parent = nullptr);
    ~MarketDataWidget();

    // Symbol management
    void addSymbol(const QString& symbol);
    void removeSymbol(const QString& symbol);
    
    // Data updates
    void updateMarketData(const QString& symbol, const MarketData& data);
    void updateTickData(const QString& symbol, double bid, double ask, double last, double volume);
    void clearData();
    
    // Configuration
    void setUpdateInterval(int milliseconds);
    void startUpdates();
    void stopUpdates();

signals:
    void symbolSelected(const QString& symbol);
    void marketDataUpdated();

private slots:
    void onUpdateTimer();
    void onRefreshClicked();
    void onSymbolSelected(const QString& symbol);

private:
    void setupUI();
    void setupTable();
    void connectSignals();
    void updateTable();
    void initializeSampleData();

    // UI components
    QComboBox* symbolSelector_;
    QPushButton* refreshButton_;
    QTableWidget* dataTable_;
    QTimer* updateTimer_;
    QLabel* statusLabel_;
    QLabel* lastUpdateTime_;
    
    // Data
    QStringList symbols_;
    QMap<QString, MarketData> marketData_;
};

} // namespace MasterMind

#endif // MASTERMIND_MARKET_DATA_WIDGET_H 