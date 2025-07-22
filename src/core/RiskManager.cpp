#include "core/RiskManager.h"
#include <iostream>

namespace MasterMind {

RiskManager::RiskManager() 
    : currentStatus_(RiskStatus::NORMAL), paperMode_(false), emergencyStop_(false),
      equityHighWaterMark_(0), currentDrawdown_(0), maxDrawdown_(0),
      dailyStartBalance_(0), dailyPnL_(0), dailyRiskUsed_(0),
      consecutiveLosses_(0), consecutiveWins_(0), maxConsecutiveLosses_(0),
      totalTrades_(0), profitableTrades_(0) {
    
    // Initialize with default parameters
    params_ = RiskParameters();
    
    // Initialize current counter
    currentCounter_ = TradingCounter();
    
    std::cout << "RiskManager initialized" << std::endl;
}

RiskManager::~RiskManager() = default;

bool RiskManager::initialize(const RiskParameters& params) {
    std::lock_guard<std::mutex> lock(paramsMutex_);
    params_ = params;
    paperMode_ = params.paperTradingMode;
    
    std::cout << "RiskManager initialized with parameters" << std::endl;
    return true;
}

void RiskManager::updateRiskParameters(const RiskParameters& params) {
    std::lock_guard<std::mutex> lock(paramsMutex_);
    params_ = params;
    std::cout << "Risk parameters updated" << std::endl;
}

RiskParameters RiskManager::getRiskParameters() const {
    std::lock_guard<std::mutex> lock(paramsMutex_);
    return params_;
}

double RiskManager::calculatePositionSize(const Symbol& symbol, 
                                        const TradingSignal& signal,
                                        const AccountInfo& account,
                                        const InstrumentSpec& instrument) const {
    
    // Simple position sizing based on fixed percentage of equity
    double riskAmount = account.equity * params_.dailyRiskPercent;
    double stopDistance = std::abs(signal.entryPrice - signal.stopLoss);
    
    if (stopDistance <= 0) {
        return 0;
    }
    
    double positionSize = riskAmount / (stopDistance * instrument.tickValue);
    
    // Apply minimum and maximum limits
    positionSize = std::max(positionSize, params_.minLotSize);
    positionSize = std::min(positionSize, account.equity * 0.1); // Max 10% of equity
    
    std::cout << "Calculated position size: " << positionSize << " for " << symbol << std::endl;
    return positionSize;
}

Volume RiskManager::calculateLotSize(const Symbol& symbol,
                                   Price entryPrice,
                                   Price stopLoss,
                                   double riskAmount,
                                   const InstrumentSpec& instrument) const {
    
    double stopDistance = std::abs(entryPrice - stopLoss);
    if (stopDistance <= 0) {
        return 0;
    }
    
    Volume lotSize = riskAmount / (stopDistance * instrument.tickValue);
    return std::max(lotSize, params_.minLotSize);
}

bool RiskManager::validateOrder(const Order& order,
                              const AccountInfo& account,
                              const std::vector<Position>& positions) const {
    
    // Basic validation checks
    if (emergencyStop_) {
        std::cout << "Order rejected: Emergency stop active" << std::endl;
        return false;
    }
    
    if (!isWithinDailyRiskLimit(order, account)) {
        std::cout << "Order rejected: Daily risk limit exceeded" << std::endl;
        return false;
    }
    
    if (!isWithinDrawdownLimit(account)) {
        std::cout << "Order rejected: Drawdown limit exceeded" << std::endl;
        return false;
    }
    
    return true;
}

bool RiskManager::isWithinDailyRiskLimit(const Order& order,
                                       const AccountInfo& account) const {
    
    double maxDailyRisk = account.equity * params_.dailyRiskPercent;
    return dailyRiskUsed_ < maxDailyRisk;
}

bool RiskManager::isWithinDrawdownLimit(const AccountInfo& account) const {
    return currentDrawdown_ < params_.maxDrawdownPercent;
}

bool RiskManager::isPositionSizeValid(const Order& order,
                                     const InstrumentSpec& instrument) const {
    return order.quantity >= params_.minLotSize;
}

RiskStatus RiskManager::getCurrentRiskStatus() const {
    return currentStatus_;
}

void RiskManager::updateRiskStatus(const AccountInfo& account,
                                 const std::vector<Position>& positions) {
    
    calculateDrawdown(account.equity);
    
    if (emergencyStop_) {
        currentStatus_ = RiskStatus::LIMIT_REACHED;
    } else if (paperMode_) {
        currentStatus_ = RiskStatus::PAPER_MODE;
    } else if (currentDrawdown_ > params_.maxDrawdownPercent * 0.8) {
        currentStatus_ = RiskStatus::WARNING;
    } else {
        currentStatus_ = RiskStatus::NORMAL;
    }
}

double RiskManager::getCurrentDrawdown() const {
    return currentDrawdown_;
}

double RiskManager::getMaxDrawdown() const {
    return maxDrawdown_;
}

double RiskManager::getDailyPnL() const {
    return dailyPnL_;
}

double RiskManager::getDailyRiskUsed() const {
    return dailyRiskUsed_;
}

bool RiskManager::shouldSwitchToPaperMode() const {
    return consecutiveLosses_ >= params_.consecutiveLossLimit;
}

bool RiskManager::shouldSwitchToLiveMode() const {
    return paperMode_ && consecutiveWins_ >= 3; // Simple criterion
}

void RiskManager::switchToPaperMode() {
    paperMode_ = true;
    currentStatus_ = RiskStatus::PAPER_MODE;
    std::cout << "Switched to paper trading mode" << std::endl;
}

void RiskManager::switchToLiveMode() {
    paperMode_ = false;
    std::cout << "Switched to live trading mode" << std::endl;
}

bool RiskManager::isPaperMode() const {
    return paperMode_;
}

void RiskManager::recordTrade(const Order& order, bool profitable) {
    totalTrades_++;
    
    if (profitable) {
        consecutiveWins_++;
        consecutiveLosses_ = 0;
        profitableTrades_++;
    } else {
        consecutiveLosses_++;
        consecutiveWins_ = 0;
        maxConsecutiveLosses_ = std::max(maxConsecutiveLosses_, consecutiveLosses_);
        
        if (shouldSwitchToPaperMode() && !paperMode_) {
            switchToPaperMode();
        }
    }
    
    std::cout << "Trade recorded: " << (profitable ? "Profit" : "Loss") 
              << ", Consecutive losses: " << consecutiveLosses_ << std::endl;
}

int RiskManager::getConsecutiveLosses() const {
    return consecutiveLosses_;
}

int RiskManager::getConsecutiveWins() const {
    return consecutiveWins_;
}

void RiskManager::resetConsecutiveCount() {
    consecutiveLosses_ = 0;
    consecutiveWins_ = 0;
}

void RiskManager::performDailyReset() {
    dailyPnL_ = 0;
    dailyRiskUsed_ = 0;
    lastDailyReset_ = std::chrono::system_clock::now();
    std::cout << "Daily reset performed" << std::endl;
}

void RiskManager::resetDailyCounters() {
    performDailyReset();
}

bool RiskManager::isDailyResetRequired() const {
    return isNewTradingDay();
}

double RiskManager::getEquityHighWaterMark() const {
    return equityHighWaterMark_;
}

void RiskManager::enableEmergencyStop() {
    emergencyStop_ = true;
    currentStatus_ = RiskStatus::LIMIT_REACHED;
    std::cout << "Emergency stop activated" << std::endl;
}

void RiskManager::disableEmergencyStop() {
    emergencyStop_ = false;
    std::cout << "Emergency stop deactivated" << std::endl;
}

bool RiskManager::isEmergencyStopActive() const {
    return emergencyStop_;
}

void RiskManager::closeAllPositions() {
    std::cout << "Closing all positions (emergency procedure)" << std::endl;
}

void RiskManager::calculateDrawdown(double currentEquity) {
    if (currentEquity > equityHighWaterMark_) {
        equityHighWaterMark_ = currentEquity;
        highWaterMarkTime_ = std::chrono::system_clock::now();
    }
    
    if (equityHighWaterMark_ > 0) {
        currentDrawdown_ = (equityHighWaterMark_ - currentEquity) / equityHighWaterMark_;
        maxDrawdown_ = std::max(maxDrawdown_, currentDrawdown_);
    }
}

bool RiskManager::isNewTradingDay() const {
    auto now = std::chrono::system_clock::now();
    auto timeSinceReset = std::chrono::duration_cast<std::chrono::hours>(now - lastDailyReset_);
    return timeSinceReset.count() >= 24;
}

// Counter management methods
void RiskManager::startNewCounter() {
    std::lock_guard<std::mutex> lock(counterMutex_);
    if (currentCounter_.isComplete || currentCounter_.ordersCount == 0) {
        currentCounter_ = TradingCounter();
        currentCounter_.counterNumber = completedCounters_.size() + 1;
        currentCounter_.startTime = std::chrono::system_clock::now();
        std::cout << "Started new counter #" << currentCounter_.counterNumber << std::endl;
    }
}

void RiskManager::addOrderToCounter(const Order& order) {
    std::lock_guard<std::mutex> lock(counterMutex_);
    currentCounter_.orders.push_back(order);
    currentCounter_.ordersCount++;
    
    if (currentCounter_.ordersCount >= params_.ordersPerCounter) {
        completeCounter();
    }
}

void RiskManager::completeCounter() {
    std::lock_guard<std::mutex> lock(counterMutex_);
    currentCounter_.isComplete = true;
    currentCounter_.endTime = std::chrono::system_clock::now();
    
    completedCounters_.push_back(currentCounter_);
    std::cout << "Counter #" << currentCounter_.counterNumber 
              << " completed with " << currentCounter_.ordersCount << " orders" << std::endl;
}

bool RiskManager::isCounterComplete() const {
    std::lock_guard<std::mutex> lock(counterMutex_);
    return currentCounter_.isComplete;
}

int RiskManager::getCurrentCounterSize() const {
    return params_.ordersPerCounter;
}

int RiskManager::getOrdersInCurrentCounter() const {
    std::lock_guard<std::mutex> lock(counterMutex_);
    return currentCounter_.ordersCount;
}

double RiskManager::getCounterPnL() const {
    std::lock_guard<std::mutex> lock(counterMutex_);
    return currentCounter_.totalPnL;
}

double RiskManager::getCapitalAfterCounter(double initialCapital) const {
    return initialCapital + getCounterPnL() - currentCounter_.totalCharges;
}

// Stub implementations for remaining methods
double RiskManager::getRiskAdjustedReturn() const { return 0.0; }
double RiskManager::getSharpeRatio() const { return 0.0; }
double RiskManager::getMaxConsecutiveLoss() const { return maxConsecutiveLosses_; }
double RiskManager::getMaxPositionSize(const Symbol& symbol, const InstrumentSpec& instrument) const { return 1000.0; }
double RiskManager::getTotalExposure(const std::vector<Position>& positions) const { return 0.0; }
double RiskManager::getSymbolExposure(const Symbol& symbol, const std::vector<Position>& positions) const { return 0.0; }
void RiskManager::setRiskAlertCallback(std::function<void(const std::string&)> callback) { riskAlertCallback_ = callback; }

} // namespace MasterMind 