#ifndef MASTERMIND_RISK_MANAGER_H
#define MASTERMIND_RISK_MANAGER_H

#include "Types.h"
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>

namespace MasterMind {

/**
 * @brief Advanced risk management system for Master Mind strategy
 * 
 * Implements sophisticated risk controls including:
 * - Dynamic position sizing based on equity and risk percentage
 * - Counter-based capital assessment (default 10 orders per counter)
 * - Paper trading fallback after consecutive losses
 * - Daily risk limits and drawdown protection
 */
class RiskManager {
public:
    /**
     * @brief Constructor
     */
    RiskManager();
    
    /**
     * @brief Destructor
     */
    ~RiskManager();
    
    // Initialization and configuration
    bool initialize(const RiskParameters& params);
    void updateRiskParameters(const RiskParameters& params);
    RiskParameters getRiskParameters() const;
    
    // Position sizing calculations
    double calculatePositionSize(const Symbol& symbol, 
                               const TradingSignal& signal,
                               const AccountInfo& account,
                               const InstrumentSpec& instrument) const;
    
    Volume calculateLotSize(const Symbol& symbol,
                          Price entryPrice,
                          Price stopLoss,
                          double riskAmount,
                          const InstrumentSpec& instrument) const;
    
    // Risk validation
    bool validateOrder(const Order& order,
                      const AccountInfo& account,
                      const std::vector<Position>& positions) const;
    
    bool isWithinDailyRiskLimit(const Order& order,
                              const AccountInfo& account) const;
    
    bool isWithinDrawdownLimit(const AccountInfo& account) const;
    
    bool isPositionSizeValid(const Order& order,
                           const InstrumentSpec& instrument) const;
    
    // Counter-based capital assessment
    void startNewCounter();
    void addOrderToCounter(const Order& order);
    void completeCounter();
    bool isCounterComplete() const;
    int getCurrentCounterSize() const;
    int getOrdersInCurrentCounter() const;
    double getCounterPnL() const;
    double getCapitalAfterCounter(double initialCapital) const;
    
    // Risk status monitoring
    RiskStatus getCurrentRiskStatus() const;
    void updateRiskStatus(const AccountInfo& account,
                         const std::vector<Position>& positions);
    
    double getCurrentDrawdown() const;
    double getMaxDrawdown() const;
    double getDailyPnL() const;
    double getDailyRiskUsed() const;
    
    // Paper trading controls
    bool shouldSwitchToPaperMode() const;
    bool shouldSwitchToLiveMode() const;
    void switchToPaperMode();
    void switchToLiveMode();
    bool isPaperMode() const;
    
    // Consecutive loss tracking
    void recordTrade(const Order& order, bool profitable);
    int getConsecutiveLosses() const;
    int getConsecutiveWins() const;
    void resetConsecutiveCount();
    
    // Daily reset and maintenance
    void performDailyReset();
    void resetDailyCounters();
    bool isDailyResetRequired() const;
    
    // Risk metrics and reporting
    double getEquityHighWaterMark() const;
    double getRiskAdjustedReturn() const;
    double getSharpeRatio() const;
    double getMaxConsecutiveLoss() const;
    
    // Position management
    double getMaxPositionSize(const Symbol& symbol,
                            const InstrumentSpec& instrument) const;
    
    double getTotalExposure(const std::vector<Position>& positions) const;
    double getSymbolExposure(const Symbol& symbol,
                           const std::vector<Position>& positions) const;
    
    // Emergency controls
    void enableEmergencyStop();
    void disableEmergencyStop();
    bool isEmergencyStopActive() const;
    void closeAllPositions();
    
    // Risk alerts and notifications
    void setRiskAlertCallback(std::function<void(const std::string&)> callback);
    
private:
    // Risk parameters
    RiskParameters params_;
    mutable std::mutex paramsMutex_;
    
    // Current risk state
    std::atomic<RiskStatus> currentStatus_;
    std::atomic<bool> paperMode_;
    std::atomic<bool> emergencyStop_;
    
    // Drawdown tracking
    double equityHighWaterMark_;
    double currentDrawdown_;
    double maxDrawdown_;
    TimePoint highWaterMarkTime_;
    
    // Daily tracking
    double dailyStartBalance_;
    double dailyPnL_;
    double dailyRiskUsed_;
    TimePoint lastDailyReset_;
    
    // Counter management
    struct TradingCounter {
        int counterNumber;
        int ordersCount;
        std::vector<Order> orders;
        double initialCapital;
        double currentCapital;
        double totalPnL;
        double totalCharges;
        TimePoint startTime;
        TimePoint endTime;
        bool isComplete;
        
        TradingCounter() : counterNumber(0), ordersCount(0), initialCapital(0),
                          currentCapital(0), totalPnL(0), totalCharges(0), 
                          isComplete(false) {}
    };
    
    TradingCounter currentCounter_;
    std::vector<TradingCounter> completedCounters_;
    mutable std::mutex counterMutex_;
    
    // Consecutive loss tracking
    int consecutiveLosses_;
    int consecutiveWins_;
    int maxConsecutiveLosses_;
    TimePoint lastTradeTime_;
    
    // Statistics
    std::vector<double> dailyReturns_;
    double totalTrades_;
    double profitableTrades_;
    
    // Callbacks
    std::function<void(const std::string&)> riskAlertCallback_;
    
    // Private methods
    void calculateDrawdown(double currentEquity);
    void updateEquityHighWaterMark(double equity);
    bool checkDailyRiskLimit(double additionalRisk) const;
    bool checkDrawdownThreshold(double currentEquity) const;
    
    // Position sizing helpers
    double calculateRiskAmount(const AccountInfo& account) const;
    double adjustForInstrumentSpecs(double rawSize,
                                  const InstrumentSpec& instrument) const;
    double applyPositionSizingRules(double calculatedSize,
                                  const InstrumentSpec& instrument) const;
    
    // Counter logic helpers
    void initializeNewCounter(double availableCapital);
    void finalizeCurrentCounter();
    double calculateCounterPerformance() const;
    bool shouldStartNextCounter() const;
    
    // Risk calculation methods
    double calculatePortfolioRisk(const std::vector<Position>& positions) const;
    double calculateCorrelationRisk(const std::vector<Position>& positions) const;
    double calculateLeverageRisk(const AccountInfo& account) const;
    
    // Alert and notification helpers
    void sendRiskAlert(const std::string& message);
    void checkRiskThresholds(const AccountInfo& account);
    void monitorRiskLimits();
    
    // Utility methods
    bool isNewTradingDay() const;
    double calculateSharpe(const std::vector<double>& returns) const;
    void updateStatistics(bool profitable, double pnl);
    void logRiskEvent(const std::string& event) const;
    
    // Validation helpers
    bool isValidRiskParameters(const RiskParameters& params) const;
    bool isOrderWithinLimits(const Order& order) const;
    
    // Emergency procedures
    void activateEmergencyProtocol();
    void executeRiskReduction();
    void notifyEmergencyStop();
};

/**
 * @brief Risk manager factory for different risk profiles
 */
class RiskManagerFactory {
public:
    static std::unique_ptr<RiskManager> createConservativeManager();
    static std::unique_ptr<RiskManager> createAggressiveManager();
    static std::unique_ptr<RiskManager> createCustomManager(const RiskParameters& params);
};

/**
 * @brief Risk assessment utilities
 */
class RiskAssessment {
public:
    static double calculateVaR(const std::vector<double>& returns, double confidence = 0.95);
    static double calculateExpectedShortfall(const std::vector<double>& returns, double confidence = 0.95);
    static double calculateMaxDrawdown(const std::vector<double>& equityCurve);
    static double calculateSharpeRatio(const std::vector<double>& returns, double riskFreeRate = 0.02);
    static double calculateVolatility(const std::vector<double>& returns);
};

} // namespace MasterMind

#endif // MASTERMIND_RISK_MANAGER_H 