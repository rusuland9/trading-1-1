#ifndef MASTERMIND_TYPES_H
#define MASTERMIND_TYPES_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <functional>

namespace MasterMind {

// Time types
using TimePoint = std::chrono::system_clock::time_point;
using Duration = std::chrono::milliseconds;

// Basic data types
using Price = double;
using Volume = double;
using OrderId = std::string;
using Symbol = std::string;
using ExchangeId = std::string;
using Currency = std::string;

// Enums
enum class OrderType {
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT,
    ICEBERG,
    PEGGED,
    HYBRID
};

enum class OrderSide {
    BUY,
    SELL
};

enum class OrderStatus {
    PENDING,
    SUBMITTED,
    PARTIALLY_FILLED,
    FILLED,
    CANCELLED,
    REJECTED,
    EXPIRED
};

enum class AssetClass {
    FOREX,
    CRYPTO,
    FUTURES,
    OPTIONS
};

enum class Exchange {
    BINANCE,
    DERIBIT,
    COINBASE,
    DELTA_EXCHANGE,
    MT4,
    MT5
};

enum class RiskStatus {
    NORMAL,
    WARNING,
    LIMIT_REACHED,
    PAPER_MODE
};

enum class PatternType {
    SETUP_1_CONSECUTIVE,
    SETUP_2_GREEN_RED_GREEN,
    NONE
};

// Market data structures
struct Tick {
    Symbol symbol;
    Price bid;
    Price ask;
    Price last;
    Volume volume;
    TimePoint timestamp;
    
    Tick() = default;
    Tick(const Symbol& sym, Price b, Price a, Price l, Volume v, TimePoint ts)
        : symbol(sym), bid(b), ask(a), last(l), volume(v), timestamp(ts) {}
};

struct OHLC {
    Price open;
    Price high;
    Price low;
    Price close;
    Volume volume;
    TimePoint timestamp;
    
    OHLC() = default;
    OHLC(Price o, Price h, Price l, Price c, Volume v, TimePoint ts)
        : open(o), high(h), low(l), close(c), volume(v), timestamp(ts) {}
};

// Renko brick structure
struct RenkoBrick {
    Price open;
    Price close;
    Price high;  // For tracking wicks
    Price low;   // For tracking wicks
    TimePoint timestamp;
    bool isUp;   // true for up brick, false for down brick
    double completionPercent; // 0.0 to 1.0 for partial brick formation
    
    RenkoBrick() : open(0), close(0), high(0), low(0), isUp(true), completionPercent(0) {}
    RenkoBrick(Price o, Price c, TimePoint ts, bool up) 
        : open(o), close(c), high(std::max(o,c)), low(std::min(o,c)), timestamp(ts), isUp(up), completionPercent(1.0) {}
};

// Order structure
struct Order {
    OrderId orderId;
    Symbol symbol;
    OrderType type;
    OrderSide side;
    Price price;
    Volume quantity;
    Volume filledQuantity;
    OrderStatus status;
    TimePoint createTime;
    TimePoint updateTime;
    ExchangeId exchange;
    std::string strategyId;
    
    // Stop loss and take profit
    Price stopLoss;
    Price takeProfit;
    
    // Advanced order parameters
    Price triggerPrice;  // For stop orders
    Volume visibleQuantity;  // For iceberg orders
    int tickOffset;  // Buffer in ticks
    
    Order() : price(0), quantity(0), filledQuantity(0), status(OrderStatus::PENDING), 
              stopLoss(0), takeProfit(0), triggerPrice(0), visibleQuantity(0), tickOffset(0) {}
};

// Position structure
struct Position {
    Symbol symbol;
    OrderSide side;
    Volume quantity;
    Price averagePrice;
    Price currentPrice;
    Price unrealizedPnL;
    Price realizedPnL;
    TimePoint openTime;
    TimePoint updateTime;
    ExchangeId exchange;
    
    Position() : quantity(0), averagePrice(0), currentPrice(0), 
                unrealizedPnL(0), realizedPnL(0) {}
    
    Price getMarketValue() const { return quantity * currentPrice; }
    bool isLong() const { return side == OrderSide::BUY; }
    bool isShort() const { return side == OrderSide::SELL; }
};

// Trading signal structure
struct TradingSignal {
    Symbol symbol;
    PatternType pattern;
    OrderSide side;
    Price entryPrice;
    Price stopLoss;
    Price takeProfit;
    Volume quantity;
    TimePoint timestamp;
    double confidence;  // 0.0 to 1.0
    std::string description;
    
    TradingSignal() : pattern(PatternType::NONE), entryPrice(0), stopLoss(0), 
                     takeProfit(0), quantity(0), confidence(0) {}
};

// Risk parameters structure
struct RiskParameters {
    double dailyRiskPercent;      // 1% or 2%
    double maxDrawdownPercent;    // Maximum allowed drawdown
    int consecutiveLossLimit;     // Switch to paper after this many losses
    double capitalUtilization;    // % of capital to use
    int ordersPerCounter;         // Default 10
    double minLotSize;           // Minimum position size
    bool paperTradingMode;       // Current mode
    
    RiskParameters() : dailyRiskPercent(0.01), maxDrawdownPercent(0.05), 
                      consecutiveLossLimit(2), capitalUtilization(1.0), 
                      ordersPerCounter(10), minLotSize(0.01), paperTradingMode(false) {}
};

// Account information
struct AccountInfo {
    double balance;
    double equity;
    double margin;
    double freeMargin;
    double marginLevel;
    Price unrealizedPnL;
    Price realizedPnL;
    Currency currency;
    TimePoint lastUpdate;
    
    AccountInfo() : balance(0), equity(0), margin(0), freeMargin(0), 
                   marginLevel(0), unrealizedPnL(0), realizedPnL(0) {}
};

// Instrument specification
struct InstrumentSpec {
    Symbol symbol;
    AssetClass assetClass;
    double tickSize;
    double tickValue;
    double contractSize;
    double marginRequirement;
    int precision;
    bool isActive;
    std::string baseAsset;
    std::string quoteAsset;
    
    InstrumentSpec() : tickSize(0.0001), tickValue(1), contractSize(1), 
                      marginRequirement(0.01), precision(5), isActive(true) {}
};

// Configuration for each symbol
struct SymbolConfig {
    Symbol symbol;
    double capitalAllocation;  // Amount allocated to this symbol
    double brickSize;         // Renko brick size
    RiskParameters riskParams;
    bool isEnabled;
    TimePoint sessionStart;
    TimePoint sessionEnd;
    
    SymbolConfig() : capitalAllocation(1000), brickSize(0.001), isEnabled(true) {}
};

// Pattern detection result
struct PatternResult {
    PatternType type;
    Symbol symbol;
    std::vector<RenkoBrick> bricks;  // Relevant bricks for the pattern
    double confidence;
    TimePoint detectionTime;
    Price suggestedEntry;
    Price suggestedStop;
    OrderSide suggestedSide;
    
    PatternResult() : type(PatternType::NONE), confidence(0), 
                     suggestedEntry(0), suggestedStop(0) {}
};

// Trading statistics
struct TradingStats {
    int totalTrades;
    int winningTrades;
    int losingTrades;
    double totalProfit;
    double totalLoss;
    double largestWin;
    double largestLoss;
    double winRate;
    double profitFactor;
    double sharpeRatio;
    int consecutiveWins;
    int consecutiveLosses;
    int currentStreak;
    TimePoint lastUpdate;
    
    TradingStats() : totalTrades(0), winningTrades(0), losingTrades(0),
                    totalProfit(0), totalLoss(0), largestWin(0), largestLoss(0),
                    winRate(0), profitFactor(0), sharpeRatio(0),
                    consecutiveWins(0), consecutiveLosses(0), currentStreak(0) {}
    
    void updateStats() {
        winRate = totalTrades > 0 ? static_cast<double>(winningTrades) / totalTrades : 0;
        profitFactor = totalLoss != 0 ? totalProfit / std::abs(totalLoss) : 0;
    }
};

// Callback function types
using TickCallback = std::function<void(const Tick&)>;
using OrderCallback = std::function<void(const Order&)>;
using SignalCallback = std::function<void(const TradingSignal&)>;

} // namespace MasterMind

#endif // MASTERMIND_TYPES_H 