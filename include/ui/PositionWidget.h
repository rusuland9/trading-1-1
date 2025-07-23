#ifndef MASTERMIND_POSITION_WIDGET_H
#define MASTERMIND_POSITION_WIDGET_H

#include <QtWidgets/QWidget>
#include <QtCore/QString>

class QTableWidget;
class QLabel;
class QPushButton;
class QTimer;

namespace MasterMind {

/**
 * @brief Widget for displaying and managing current positions
 * 
 * Shows real-time position data including P&L, exposure, and provides
 * controls for closing positions as specified in the requirements.
 */
class PositionWidget : public QWidget {
    Q_OBJECT

public:
    explicit PositionWidget(QWidget* parent = nullptr);
    ~PositionWidget();

    // Public interface for position management
    void addPosition(const QString& symbol, const QString& side, double size, double entryPrice);
    void clearPositions();

signals:
    void positionClosed(const QString& symbol, const QString& side, double size);
    void allPositionsClosed();
    void positionsRefreshRequested();
    void positionsUpdated();

private slots:
    void updatePositions();
    void onRefreshPositions();
    void onCloseAllPositions();

private:
    void setupUI();
    QWidget* createSummarySection();
    QWidget* createPositionsSection();
    QWidget* createControlsSection();
    void setupPositionsTable();
    void connectSignals();
    void initializeData();
    void addSamplePositions();
    void updateSummary();
    void recalculateTotalPnL();
    void closePosition(int row);

    // UI components
    QTableWidget* positionsTable_;
    QLabel* totalPnLLabel_;
    QLabel* totalExposureLabel_;
    QLabel* openPositionsLabel_;
    QPushButton* closeAllButton_;
    QPushButton* refreshButton_;
    QTimer* updateTimer_;

    // Data
    double totalPnL_;
    double totalExposure_;
    int openPositionsCount_;
};

} // namespace MasterMind

#endif // MASTERMIND_POSITION_WIDGET_H 