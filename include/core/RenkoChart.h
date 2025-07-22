#ifndef MASTERMIND_RENKO_CHART_H
#define MASTERMIND_RENKO_CHART_H

#include "Types.h"
#include <vector>
#include <deque>
#include <memory>
#include <mutex>

namespace MasterMind {

/**
 * @brief Real-time Renko chart implementation
 * 
 * This class handles the creation and maintenance of Renko bricks in real-time,
 * supporting partial brick formation tracking and pattern detection requirements
 * for the Master Mind strategy.
 */
class RenkoChart {
public:
    /**
     * @brief Constructor
     * @param symbol The trading symbol
     * @param brickSize The size of each Renko brick
     * @param maxBricks Maximum number of bricks to keep in memory (default: 1000)
     */
    RenkoChart(const Symbol& symbol, double brickSize, size_t maxBricks = 1000);
    
    /**
     * @brief Destructor
     */
    ~RenkoChart();
    
    // Core functionality
    void addTick(const Tick& tick);
    void addPrice(Price price, TimePoint timestamp);
    void setBrickSize(double brickSize);
    double getBrickSize() const;
    
    // Brick access methods
    std::vector<RenkoBrick> getBricks(size_t count = 0) const;
    std::vector<RenkoBrick> getLastNBricks(size_t n) const;
    RenkoBrick getLastBrick() const;
    RenkoBrick getCurrentBrick() const;  // Includes partial formation
    size_t getBrickCount() const;
    
    // Pattern detection helpers
    bool hasConsecutiveDownBricks(int count = 2) const;
    bool hasConsecutiveUpBricks(int count = 2) const;
    bool hasGreenRedGreenPattern() const;
    bool hasRedGreenRedPattern() const;
    double getPartialBrickCompletion() const;  // 0.0 to 1.0
    
    // Price level calculations
    Price getNextUpBrickLevel() const;
    Price getNextDownBrickLevel() const;
    Price getBrickHighPrice(const RenkoBrick& brick) const;
    Price getBrickLowPrice(const RenkoBrick& brick) const;
    
    // Signal generation helpers
    Price calculateSetup1EntryPrice(OrderSide side, int tickBuffer = 2) const;
    Price calculateSetup2EntryPrice(OrderSide side, int tickBuffer = 2) const;
    Price calculateStopLoss(OrderSide side, int tickBuffer = 2) const;
    
    // Brick formation validation
    bool isUpBrickForming() const;
    bool isDownBrickForming() const;
    bool isBrickComplete(double completionThreshold = 0.75) const;
    
    // Statistics and analysis
    double getAverageBrickTime() const;  // Average time to complete a brick
    int getConsecutiveUpCount() const;
    int getConsecutiveDownCount() const;
    
    // State management
    void reset();
    void clearOldBricks(size_t keepCount = 100);
    Symbol getSymbol() const;
    TimePoint getLastUpdateTime() const;
    
    // Configuration
    void setTickValue(double tickValue);
    double getTickValue() const;
    void setMaxBricks(size_t maxBricks);
    
private:
    Symbol symbol_;
    double brickSize_;
    double tickValue_;
    size_t maxBricks_;
    
    // Brick storage
    std::deque<RenkoBrick> bricks_;
    RenkoBrick currentBrick_;  // Brick currently being formed
    bool currentBrickInitialized_;
    
    // Price tracking
    Price lastPrice_;
    Price highestPrice_;  // Highest price since last brick
    Price lowestPrice_;   // Lowest price since last brick
    TimePoint lastUpdate_;
    
    // Pattern detection state
    mutable std::mutex dataMutex_;
    
    // Statistics
    std::vector<Duration> brickFormationTimes_;
    
    // Private methods
    void updateCurrentBrick(Price price, TimePoint timestamp);
    bool tryFormUpBrick(Price price, TimePoint timestamp);
    bool tryFormDownBrick(Price price, TimePoint timestamp);
    void finalizeBrick(const RenkoBrick& brick);
    void initializeCurrentBrick(Price price, TimePoint timestamp);
    
    // Pattern detection helpers
    bool checkPattern(const std::vector<bool>& pattern) const;  // true = up, false = down
    std::vector<bool> getLastNBrickDirections(size_t n) const;
    
    // Validation methods
    bool isValidPrice(Price price) const;
    bool isValidBrickSize(double size) const;
    
    // Utility methods
    Price roundToBrickLevel(Price price, bool roundUp) const;
    Duration calculateBrickTime(const RenkoBrick& brick) const;
    void updateStatistics(const RenkoBrick& brick);
    
    // Memory management
    void maintainMaxBricks();
};

/**
 * @brief Renko chart factory for creating charts with different configurations
 */
class RenkoChartFactory {
public:
    static std::unique_ptr<RenkoChart> createChart(
        const Symbol& symbol, 
        AssetClass assetClass,
        double brickSize = 0.0
    );
    
    static double calculateOptimalBrickSize(
        const Symbol& symbol,
        const std::vector<OHLC>& historicalData,
        int periods = 20
    );
    
    static double getDefaultBrickSize(AssetClass assetClass);
};

} // namespace MasterMind

#endif // MASTERMIND_RENKO_CHART_H 