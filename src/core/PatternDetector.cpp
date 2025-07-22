#include "core/PatternDetector.h"
#include <iostream>

namespace MasterMind {

PatternDetector::PatternDetector() 
    : minConfidence_(0.7), partialBrickThreshold_(0.75), tickBuffer_(2),
      setup1Enabled_(true), setup2Enabled_(true) {
    
    std::cout << "PatternDetector initialized" << std::endl;
}

PatternDetector::~PatternDetector() = default;

std::vector<PatternResult> PatternDetector::detectPatterns(const RenkoChart& chart) {
    std::vector<PatternResult> results;
    
    if (setup1Enabled_) {
        auto setup1Result = detectSetup1Pattern(chart);
        if (setup1Result.type != PatternType::NONE) {
            results.push_back(setup1Result);
        }
    }
    
    if (setup2Enabled_) {
        auto setup2Result = detectSetup2Pattern(chart);
        if (setup2Result.type != PatternType::NONE) {
            results.push_back(setup2Result);
        }
    }
    
    return results;
}

PatternResult PatternDetector::detectSetup1Pattern(const RenkoChart& chart) {
    PatternResult result;
    result.type = PatternType::NONE;
    result.symbol = chart.getSymbol();
    result.detectionTime = std::chrono::system_clock::now();
    
    // Get recent bricks for analysis
    auto recentBricks = chart.getLastNBricks(5);
    if (recentBricks.size() < 3) {
        return result;
    }
    
    // Simple Setup 1 detection logic
    // Check for two consecutive down bricks followed by partial up brick
    if (recentBricks.size() >= 3) {
        bool hasConsecutiveDown = !recentBricks[recentBricks.size()-3].isUp && 
                                 !recentBricks[recentBricks.size()-2].isUp;
        
        auto currentBrick = chart.getCurrentBrick();
        bool hasPartialUp = currentBrick.isUp && 
                           currentBrick.completionPercent >= partialBrickThreshold_;
        
        if (hasConsecutiveDown && hasPartialUp) {
            result.type = PatternType::SETUP_1_CONSECUTIVE;
            result.confidence = 0.8;
            result.suggestedSide = OrderSide::BUY;
            result.suggestedEntry = chart.calculateSetup1EntryPrice(OrderSide::BUY, tickBuffer_);
            result.suggestedStop = chart.calculateStopLoss(OrderSide::BUY, tickBuffer_);
            result.bricks = recentBricks;
            
            std::cout << "Setup 1 pattern detected for " << chart.getSymbol() << std::endl;
        }
    }
    
    return result;
}

PatternResult PatternDetector::detectSetup2Pattern(const RenkoChart& chart) {
    PatternResult result;
    result.type = PatternType::NONE;
    result.symbol = chart.getSymbol();
    result.detectionTime = std::chrono::system_clock::now();
    
    // Get recent bricks for analysis
    auto recentBricks = chart.getLastNBricks(5);
    if (recentBricks.size() < 3) {
        return result;
    }
    
    // Simple Setup 2 detection logic (Green-Red-Green pattern)
    if (recentBricks.size() >= 3) {
        bool hasGreenRedGreen = recentBricks[recentBricks.size()-3].isUp && 
                               !recentBricks[recentBricks.size()-2].isUp &&
                               recentBricks[recentBricks.size()-1].isUp;
        
        auto currentBrick = chart.getCurrentBrick();
        bool isPartiallyFormed = currentBrick.completionPercent >= partialBrickThreshold_;
        
        if (hasGreenRedGreen && isPartiallyFormed) {
            result.type = PatternType::SETUP_2_GREEN_RED_GREEN;
            result.confidence = 0.75;
            result.suggestedSide = OrderSide::BUY;
            result.suggestedEntry = chart.calculateSetup2EntryPrice(OrderSide::BUY, tickBuffer_);
            result.suggestedStop = chart.calculateStopLoss(OrderSide::BUY, tickBuffer_);
            result.bricks = recentBricks;
            
            std::cout << "Setup 2 pattern detected for " << chart.getSymbol() << std::endl;
        }
    }
    
    return result;
}

bool PatternDetector::isSetup1Triggered(const RenkoChart& chart, OrderSide& suggestedSide) {
    auto result = detectSetup1Pattern(chart);
    if (result.type != PatternType::NONE) {
        suggestedSide = result.suggestedSide;
        return true;
    }
    return false;
}

bool PatternDetector::isSetup2Triggered(const RenkoChart& chart, OrderSide& suggestedSide) {
    auto result = detectSetup2Pattern(chart);
    if (result.type != PatternType::NONE) {
        suggestedSide = result.suggestedSide;
        return true;
    }
    return false;
}

TradingSignal PatternDetector::generateSignalFromPattern(const PatternResult& pattern, 
                                                       const RenkoChart& chart,
                                                       const SymbolConfig& config) {
    TradingSignal signal;
    
    if (pattern.type == PatternType::NONE) {
        return signal;
    }
    
    signal.symbol = pattern.symbol;
    signal.pattern = pattern.type;
    signal.side = pattern.suggestedSide;
    signal.entryPrice = pattern.suggestedEntry;
    signal.stopLoss = pattern.suggestedStop;
    signal.timestamp = pattern.detectionTime;
    signal.confidence = pattern.confidence;
    
    // Calculate take profit based on risk-reward ratio
    double riskDistance = std::abs(signal.entryPrice - signal.stopLoss);
    if (signal.side == OrderSide::BUY) {
        signal.takeProfit = signal.entryPrice + (riskDistance * 2.0); // 2:1 reward:risk
    } else {
        signal.takeProfit = signal.entryPrice - (riskDistance * 2.0);
    }
    
    // Simple quantity calculation (will be refined by risk manager)
    signal.quantity = config.riskParams.minLotSize;
    
    signal.description = "Pattern: " + patternTypeToString(pattern.type) + 
                        ", Confidence: " + std::to_string(pattern.confidence);
    
    std::cout << "Generated trading signal for " << signal.symbol 
              << " (" << signal.description << ")" << std::endl;
    
    return signal;
}

void PatternDetector::setMinConfidence(double confidence) {
    minConfidence_ = std::max(0.0, std::min(1.0, confidence));
}

void PatternDetector::setPartialBrickThreshold(double threshold) {
    partialBrickThreshold_ = std::max(0.5, std::min(1.0, threshold));
}

void PatternDetector::setTickBuffer(int ticks) {
    tickBuffer_ = std::max(1, ticks);
}

void PatternDetector::enableSetup1(bool enable) {
    setup1Enabled_ = enable;
    std::cout << "Setup 1 " << (enable ? "enabled" : "disabled") << std::endl;
}

void PatternDetector::enableSetup2(bool enable) {
    setup2Enabled_ = enable;
    std::cout << "Setup 2 " << (enable ? "enabled" : "disabled") << std::endl;
}

bool PatternDetector::isPatternActive(const Symbol& symbol) const {
    return activePatterns_.find(symbol) != activePatterns_.end();
}

PatternType PatternDetector::getActivePattern(const Symbol& symbol) const {
    auto it = activePatterns_.find(symbol);
    return (it != activePatterns_.end()) ? it->second : PatternType::NONE;
}

void PatternDetector::clearPatternState(const Symbol& symbol) {
    activePatterns_.erase(symbol);
    patternStartTime_.erase(symbol);
}

void PatternDetector::updatePatternStats(const PatternResult& pattern, bool successful) {
    auto& stats = patternStats_[pattern.type];
    stats.totalCount++;
    
    if (successful) {
        stats.successCount++;
    }
    
    stats.successRate = (stats.totalCount > 0) ? 
        static_cast<double>(stats.successCount) / stats.totalCount : 0.0;
    stats.lastUpdate = std::chrono::system_clock::now();
    
    std::cout << "Pattern stats updated for " << patternTypeToString(pattern.type)
              << ": " << stats.successCount << "/" << stats.totalCount 
              << " (" << (stats.successRate * 100) << "%)" << std::endl;
}

double PatternDetector::getPatternSuccessRate(PatternType type) const {
    auto it = patternStats_.find(type);
    return (it != patternStats_.end()) ? it->second.successRate : 0.0;
}

int PatternDetector::getPatternCount(PatternType type) const {
    auto it = patternStats_.find(type);
    return (it != patternStats_.end()) ? it->second.totalCount : 0;
}

std::string PatternDetector::patternTypeToString(PatternType type) const {
    switch (type) {
        case PatternType::SETUP_1_CONSECUTIVE: return "Setup 1 (Consecutive)";
        case PatternType::SETUP_2_GREEN_RED_GREEN: return "Setup 2 (Green-Red-Green)";
        case PatternType::NONE: return "None";
        default: return "Unknown";
    }
}

void PatternDetector::logPatternDetection(const PatternResult& pattern) const {
    std::cout << "Pattern detected: " << patternTypeToString(pattern.type)
              << " for " << pattern.symbol 
              << " with confidence " << pattern.confidence << std::endl;
}

// Stub implementations for other methods
bool PatternDetector::validateSetup1(const std::vector<RenkoBrick>& bricks, double partialCompletion) {
    return bricks.size() >= 3 && partialCompletion >= partialBrickThreshold_;
}

bool PatternDetector::validateSetup2(const std::vector<RenkoBrick>& bricks, double partialCompletion) {
    return bricks.size() >= 3 && partialCompletion >= partialBrickThreshold_;
}

Price PatternDetector::calculateEntryPrice(const PatternResult& pattern, 
                                         const RenkoChart& chart, 
                                         OrderSide side) const {
    return pattern.suggestedEntry;
}

Price PatternDetector::calculateStopLoss(const PatternResult& pattern, 
                                        const RenkoChart& chart, 
                                        OrderSide side) const {
    return pattern.suggestedStop;
}

Price PatternDetector::calculateTakeProfit(const PatternResult& pattern, 
                                         const RenkoChart& chart, 
                                         OrderSide side,
                                         double riskRewardRatio) const {
    double risk = std::abs(pattern.suggestedEntry - pattern.suggestedStop);
    if (side == OrderSide::BUY) {
        return pattern.suggestedEntry + (risk * riskRewardRatio);
    } else {
        return pattern.suggestedEntry - (risk * riskRewardRatio);
    }
}

OrderSide PatternDetector::getOppositeDirection(OrderSide side) const {
    return (side == OrderSide::BUY) ? OrderSide::SELL : OrderSide::BUY;
}

} // namespace MasterMind 