#ifndef MASTERMIND_TRADING_ENGINE_H
#define MASTERMIND_TRADING_ENGINE_H

#include "Types.h"
#include "api/ExchangeAPI.h"
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace MasterMind {

// Forward declarations
class RenkoChart;
class PatternDetector;
class OrderManager;
class RiskManager;
class ConfigManager;
class Logger;
class DatabaseManager;

/**
 * @brief Main trading engine that coordinates all components
 * 
 * The TradingEngine is the central orchestrator of the Master Mind trading system.
 * It manages market data processing, pattern detection, signal generation,
 * risk management, and order execution across multiple exchanges.
 */
class TradingEngine {
public:
    /**
     * @brief Constructor
     * @param configFile Path to the configuration file
     */
    explicit TradingEngine(const std::string& configFile);
    
    /**
     * @brief Destructor
     */
    ~TradingEngine();
    
    // Core lifecycle methods
    bool initialize();
    bool start();
    void stop();
    bool isRunning() const;
    
    // Configuration management
    bool loadConfiguration(const std::string& configFile);
    bool reloadConfiguration();
    void addSymbol(const SymbolConfig& config);
    void removeSymbol(const Symbol& symbol);
    void updateSymbolConfig(const SymbolConfig& config);
    
    // Market data handling
    void onTick(const Tick& tick);
    void onOHLC(const OHLC& ohlc);
    void processMarketData();
    
    // Signal and trading methods
    void onTradingSignal(const TradingSignal& signal);
    bool placeOrder(const Order& order);
    bool cancelOrder(const OrderId& orderId);
    bool modifyOrder(const OrderId& orderId, const Order& newOrder);
    
    // Risk management
    RiskStatus getRiskStatus() const;
    double getCurrentDrawdown() const;
    bool isWithinRiskLimits(const Order& order) const;
    void switchToPaperMode();
    void switchToLiveMode();
    
    // Position management
    std::vector<Position> getPositions(const Symbol& symbol = "") const;
    Position getPosition(const Symbol& symbol) const;
    double getUnrealizedPnL() const;
    double getRealizedPnL() const;
    
    // Account information
    AccountInfo getAccountInfo() const;
    TradingStats getTradingStats() const;
    
    // Exchange management
    bool addExchange(std::unique_ptr<ExchangeAPI> exchange);
    ExchangeAPI* getExchange(Exchange exchangeType) const;
    std::vector<Exchange> getActiveExchanges() const;
    
    // Event callbacks
    void setTickCallback(TickCallback callback);
    void setOrderCallback(OrderCallback callback);
    void setSignalCallback(SignalCallback callback);
    
    // Logging and monitoring
    void enableAuditTrail(bool enable);
    std::vector<std::string> getLogEntries(int count = 100) const;
    void exportTradingReport(const std::string& filename) const;
    
    // Paper trading controls
    bool isPaperMode() const;
    void setPaperMode(bool enable);
    
    // Session management
    bool isWithinTradingSession(const Symbol& symbol) const;
    void setTradingSession(const Symbol& symbol, TimePoint start, TimePoint end);
    
private:
    // Core components
    std::unique_ptr<ConfigManager> configManager_;
    std::unique_ptr<OrderManager> orderManager_;
    std::unique_ptr<RiskManager> riskManager_;
    std::unique_ptr<DatabaseManager> databaseManager_;
    // Removed logger_ member - using singleton instead
    
    // Pattern detection and charting
    std::unordered_map<Symbol, std::unique_ptr<RenkoChart>> renkoCharts_;
    std::unique_ptr<PatternDetector> patternDetector_;
    
    // Exchange APIs
    std::unordered_map<Exchange, std::unique_ptr<ExchangeAPI>> exchanges_;
    
    // Threading and synchronization
    std::thread marketDataThread_;
    std::thread patternDetectionThread_;
    std::thread orderProcessingThread_;
    std::atomic<bool> running_;
    mutable std::mutex dataMutex_;
    mutable std::mutex orderMutex_;
    std::condition_variable cv_;
    
    // Market data queue
    std::queue<Tick> tickQueue_;
    std::queue<OHLC> ohlcQueue_;
    mutable std::mutex queueMutex_;
    
    // Configuration
    std::unordered_map<Symbol, SymbolConfig> symbolConfigs_;
    std::string configFilePath_;
    
    // State tracking
    std::atomic<RiskStatus> riskStatus_;
    std::atomic<bool> paperMode_;
    std::atomic<double> currentDrawdown_;
    
    // Callbacks
    TickCallback tickCallback_;
    OrderCallback orderCallback_;
    SignalCallback signalCallback_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    TradingStats tradingStats_;
    
    // Private methods
    void marketDataWorker();
    void patternDetectionWorker();
    void orderProcessingWorker();
    
    void processTickQueue();
    void processOHLCQueue();
    void updateRenkoCharts(const Tick& tick);
    void detectPatterns();
    void executeStrategy();
    
    void updateAccountInfo();
    void updatePositions();
    void updateRiskStatus();
    void updateTradingStats();
    
    bool validateOrder(const Order& order) const;
    OrderId generateOrderId() const;
    void logTrade(const Order& order);
    void notifyOrderUpdate(const Order& order);
    
    // Risk management helpers
    double calculatePositionSize(const Symbol& symbol, const TradingSignal& signal) const;
    bool checkDailyRiskLimit() const;
    bool checkDrawdownLimit() const;
    void handleConsecutiveLosses();
    
    // Session management helpers
    bool isMarketOpen(const Symbol& symbol) const;
    TimePoint getNextTradingSession(const Symbol& symbol) const;
    
    // Configuration validation
    bool validateConfiguration() const;
    void applyConfiguration();
    
    // Cleanup methods
    void cleanup();
    void stopAllThreads();
};

} // namespace MasterMind

#endif // MASTERMIND_TRADING_ENGINE_H 