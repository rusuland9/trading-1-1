#ifndef MASTERMIND_PATTERN_DETECTOR_H
#define MASTERMIND_PATTERN_DETECTOR_H

#include "Types.h"
#include "RenkoChart.h"
#include <memory>
#include <vector>
#include <functional>

namespace MasterMind {

/**
 * @brief Pattern detection engine for Master Mind strategy
 * 
 * Implements the two main setups:
 * Setup 1: Two consecutive down bricks + 75% up brick formation
 * Setup 2: Green-Red-Green (or Red-Green-Red) pattern with 75% completion
 */
class PatternDetector {
public:
    /**
     * @brief Constructor
     */
    PatternDetector();
    
    /**
     * @brief Destructor
     */
    ~PatternDetector();
    
    // Main pattern detection methods
    std::vector<PatternResult> detectPatterns(const RenkoChart& chart);
    PatternResult detectSetup1Pattern(const RenkoChart& chart);
    PatternResult detectSetup2Pattern(const RenkoChart& chart);
    
    // Individual pattern checks
    bool isSetup1Triggered(const RenkoChart& chart, OrderSide& suggestedSide);
    bool isSetup2Triggered(const RenkoChart& chart, OrderSide& suggestedSide);
    
    // Pattern validation
    bool validateSetup1(const std::vector<RenkoBrick>& bricks, double partialCompletion);
    bool validateSetup2(const std::vector<RenkoBrick>& bricks, double partialCompletion);
    
    // Configuration
    void setMinConfidence(double confidence);  // 0.0 to 1.0
    void setPartialBrickThreshold(double threshold);  // Default 0.75 (75%)
    void setTickBuffer(int ticks);  // Default 2 ticks
    void enableSetup1(bool enable);
    void enableSetup2(bool enable);
    
    // Pattern state tracking
    bool isPatternActive(const Symbol& symbol) const;
    PatternType getActivePattern(const Symbol& symbol) const;
    void clearPatternState(const Symbol& symbol);
    
    // Signal generation
    TradingSignal generateSignalFromPattern(const PatternResult& pattern, 
                                          const RenkoChart& chart,
                                          const SymbolConfig& config);
    
    // Entry and exit price calculations
    Price calculateEntryPrice(const PatternResult& pattern, 
                            const RenkoChart& chart, 
                            OrderSide side) const;
    Price calculateStopLoss(const PatternResult& pattern, 
                          const RenkoChart& chart, 
                          OrderSide side) const;
    Price calculateTakeProfit(const PatternResult& pattern, 
                            const RenkoChart& chart, 
                            OrderSide side,
                            double riskRewardRatio = 2.0) const;
    
    // Pattern statistics
    void updatePatternStats(const PatternResult& pattern, bool successful);
    double getPatternSuccessRate(PatternType type) const;
    int getPatternCount(PatternType type) const;
    
private:
    // Configuration parameters
    double minConfidence_;
    double partialBrickThreshold_;
    int tickBuffer_;
    bool setup1Enabled_;
    bool setup2Enabled_;
    
    // Pattern state tracking
    std::unordered_map<Symbol, PatternType> activePatterns_;
    std::unordered_map<Symbol, TimePoint> patternStartTime_;
    
    // Pattern statistics
    struct PatternStats {
        int totalCount = 0;
        int successCount = 0;
        double successRate = 0.0;
        TimePoint lastUpdate;
    };
    std::unordered_map<PatternType, PatternStats> patternStats_;
    
    // Setup 1 detection methods
    bool detectConsecutiveDownUp(const std::vector<RenkoBrick>& bricks, 
                                 double partialCompletion) const;
    bool detectConsecutiveUpDown(const std::vector<RenkoBrick>& bricks, 
                                double partialCompletion) const;
    
    // Setup 2 detection methods
    bool detectGreenRedGreen(const std::vector<RenkoBrick>& bricks, 
                            double partialCompletion) const;
    bool detectRedGreenRed(const std::vector<RenkoBrick>& bricks, 
                          double partialCompletion) const;
    
    // Pattern validation helpers
    bool isValidBrickSequence(const std::vector<RenkoBrick>& bricks) const;
    bool isPartialBrickValid(const RenkoBrick& brick, double threshold) const;
    bool hasMinimumBricks(const std::vector<RenkoBrick>& bricks, size_t minCount) const;
    
    // Price calculation helpers
    Price getWedgeHigh(const std::vector<RenkoBrick>& bricks) const;
    Price getWedgeLow(const std::vector<RenkoBrick>& bricks) const;
    Price getChannelHigh(const std::vector<RenkoBrick>& bricks) const;
    Price getChannelLow(const std::vector<RenkoBrick>& bricks) const;
    
    // Signal strength calculation
    double calculateSignalStrength(const PatternResult& pattern, 
                                  const RenkoChart& chart) const;
    double calculateTrendStrength(const std::vector<RenkoBrick>& bricks) const;
    double calculateVolumeConfirmation(const std::vector<RenkoBrick>& bricks) const;
    
    // Pattern timing validation
    bool isPatternTimingValid(const PatternResult& pattern) const;
    bool isWithinSessionTiming(const PatternResult& pattern) const;
    
    // Risk assessment for patterns
    double assessPatternRisk(const PatternResult& pattern, 
                           const RenkoChart& chart) const;
    bool isPatternRiskAcceptable(const PatternResult& pattern) const;
    
    // Utility methods
    OrderSide getOppositeDirection(OrderSide side) const;
    std::string patternTypeToString(PatternType type) const;
    void logPatternDetection(const PatternResult& pattern) const;
};

/**
 * @brief Pattern detector factory for creating configured detectors
 */
class PatternDetectorFactory {
public:
    static std::unique_ptr<PatternDetector> createDetector(
        double minConfidence = 0.7,
        double partialThreshold = 0.75,
        int tickBuffer = 2
    );
    
    static std::unique_ptr<PatternDetector> createConservativeDetector();
    static std::unique_ptr<PatternDetector> createAggressiveDetector();
};

/**
 * @brief Pattern backtesting utility
 */
class PatternBacktester {
public:
    struct BacktestResult {
        PatternType pattern;
        int totalSignals;
        int profitableSignals;
        double winRate;
        double avgProfit;
        double avgLoss;
        double profitFactor;
        double maxDrawdown;
    };
    
    static BacktestResult backtestPattern(
        PatternType pattern,
        const std::vector<OHLC>& historicalData,
        const SymbolConfig& config
    );
    
    static std::vector<BacktestResult> backtestAllPatterns(
        const std::vector<OHLC>& historicalData,
        const SymbolConfig& config
    );
};

} // namespace MasterMind

#endif // MASTERMIND_PATTERN_DETECTOR_H 