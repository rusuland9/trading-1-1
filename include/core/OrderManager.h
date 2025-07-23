#ifndef MASTERMIND_ORDER_MANAGER_H
#define MASTERMIND_ORDER_MANAGER_H

#include "Types.h"
#include "api/ExchangeAPI.h"
#include <memory>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace MasterMind {

// Forward declarations
class Logger;

/**
 * @brief Advanced order management system for Master Mind strategy
 * 
 * Handles complex order types including:
 * - Hybrid orders (combination of iceberg and pegged orders)
 * - Stop loss with tick buffer management
 * - Cross-exchange order routing
 * - Real-time order status tracking
 * - Slippage minimization
 */
class OrderManager {
public:
    /**
     * @brief Constructor
     */
    OrderManager();
    
    /**
     * @brief Destructor
     */
    ~OrderManager();
    
    // Initialization and lifecycle
    bool initialize();
    void start();
    void stop();
    bool isRunning() const;
    
    // Order submission and management
    OrderId submitOrder(const Order& order);
    bool cancelOrder(const OrderId& orderId);
    bool modifyOrder(const OrderId& orderId, const Order& newOrder);
    
    // Advanced order types
    OrderId submitHybridOrder(const Order& order, 
                            Volume icebergQuantity,
                            Price pegOffset);
    
    OrderId submitStopOrder(const Order& order,
                          Price triggerPrice,
                          int tickBuffer = 2);
    
    OrderId submitTrailingStop(const Order& order,
                             Price trailAmount,
                             bool usePercent = false);
    
    // Order status and tracking
    Order getOrder(const OrderId& orderId) const;
    std::vector<Order> getActiveOrders() const;
    std::vector<Order> getOrderHistory(const Symbol& symbol = "") const;
    OrderStatus getOrderStatus(const OrderId& orderId) const;
    
    // Position-based order management
    bool setStopLoss(const Symbol& symbol, Price stopPrice);
    bool setTakeProfit(const Symbol& symbol, Price targetPrice);
    bool updateTrailingStop(const Symbol& symbol, Price newTrailPrice);
    
    // Exchange routing
    void addExchange(Exchange exchange, std::unique_ptr<ExchangeAPI> api);
    Exchange getBestExchange(const Symbol& symbol, OrderSide side, Volume quantity) const;
    bool routeOrder(Order& order);
    
    // Order execution strategies
    void setExecutionStrategy(const Symbol& symbol, const std::string& strategy);
    void enableSmartRouting(bool enable);
    void setSlippageThreshold(double maxSlippagePercent);
    
    // Real-time updates
    void onOrderUpdate(const Order& order);
    void onFillUpdate(const OrderId& orderId, Volume fillQuantity, Price fillPrice);
    void onOrderRejected(const OrderId& orderId, const std::string& reason);
    
    // Risk integration
    void setRiskValidationCallback(std::function<bool(const Order&)> callback);
    void enableRiskValidation(bool enable);
    
    // Callbacks and events
    void setOrderCallback(OrderCallback callback);
    void setFillCallback(std::function<void(const OrderId&, Volume, Price)> callback);
    void setRejectionCallback(std::function<void(const OrderId&, const std::string&)> callback);
    
    // Statistics and monitoring
    double getAverageSlippage(const Symbol& symbol) const;
    double getFillRate() const;
    int getActiveOrderCount() const;
    std::vector<std::string> getExecutionReport() const;
    
private:
    // Order storage and tracking
    std::unordered_map<OrderId, Order> activeOrders_;
    std::unordered_map<OrderId, Order> orderHistory_;
    std::queue<Order> orderQueue_;
    mutable std::mutex ordersMutex_;
    
    // Exchange management
    std::unordered_map<Exchange, std::unique_ptr<ExchangeAPI>> exchanges_;
    std::unordered_map<Symbol, Exchange> symbolExchangeMapping_;
    
    // Threading and processing
    std::thread orderProcessingThread_;
    std::thread statusUpdateThread_;
    std::atomic<bool> running_;
    std::condition_variable orderCV_;
    
    // Configuration
    bool smartRoutingEnabled_;
    double maxSlippagePercent_;
    bool riskValidationEnabled_;
    std::unordered_map<Symbol, std::string> executionStrategies_;
    
    // Callbacks
    OrderCallback orderCallback_;
    std::function<void(const OrderId&, Volume, Price)> fillCallback_;
    std::function<void(const OrderId&, const std::string&)> rejectionCallback_;
    std::function<bool(const Order&)> riskValidationCallback_;
    
    // Statistics
    struct ExecutionStats {
        double totalSlippage = 0.0;
        int totalOrders = 0;
        int filledOrders = 0;
        int rejectedOrders = 0;
        double avgFillTime = 0.0;
        std::vector<double> slippageHistory;
    };
    std::unordered_map<Symbol, ExecutionStats> executionStats_;
    
    // Stop loss and trailing stop management
    struct StopLossInfo {
        OrderId orderId;
        Symbol symbol;
        Price stopPrice;
        Price trailAmount;
        bool isTrailing;
        bool usePercent;
        TimePoint lastUpdate;
    };
    std::unordered_map<Symbol, StopLossInfo> stopLossOrders_;
    
    // Hybrid order management
    struct HybridOrderInfo {
        OrderId parentOrderId;
        Volume totalQuantity;
        Volume visibleQuantity;
        Price pegOffset;
        std::vector<OrderId> childOrders;
        int currentSlice;
    };
    std::unordered_map<OrderId, HybridOrderInfo> hybridOrders_;
    
    // Private methods - Order processing
    void orderProcessingWorker();
    void statusUpdateWorker();
    void processOrderQueue();
    void processOrder(const Order& order);
    bool validateOrder(const Order& order) const;
    OrderId generateOrderId() const;
    
    // Order execution methods
    bool executeMarketOrder(Order& order);
    bool executeLimitOrder(Order& order);
    bool executeStopOrder(Order& order);
    bool executeHybridOrder(Order& order);
    
    // Exchange routing logic
    Exchange selectOptimalExchange(const Order& order) const;
    double calculateExecutionCost(const Order& order, Exchange exchange) const;
    bool isExchangeAvailable(Exchange exchange) const;
    
    // Slippage management
    double calculateSlippage(const Order& order, Price fillPrice) const;
    void updateSlippageStats(const Symbol& symbol, double slippage);
    bool isSlippageAcceptable(double slippage) const;
    
    // Stop loss management
    void updateTrailingStops(const Symbol& symbol, Price currentPrice);
    void triggerStopLoss(const Symbol& symbol);
    bool shouldUpdateTrailingStop(const StopLossInfo& info, Price currentPrice) const;
    
    // Hybrid order management
    void manageHybridOrder(const HybridOrderInfo& info);
    void submitNextSlice(const HybridOrderInfo& info);
    bool isHybridOrderComplete(const OrderId& orderId) const;
    
    // Order state management
    void updateOrderStatus(const OrderId& orderId, OrderStatus status);
    void moveToHistory(const OrderId& orderId);
    void cleanupExpiredOrders();
    
    // Risk and validation
    bool performRiskValidation(const Order& order) const;
    bool validateOrderParameters(const Order& order) const;
    bool checkMarginRequirements(const Order& order) const;
    
    // Utility methods
    void logOrderEvent(const OrderId& orderId, const std::string& event) const;
    void notifyOrderUpdate(const Order& order);
    void updateExecutionStats(const Order& order);
    
    // Cleanup and maintenance
    void cleanup();
    void stopAllThreads();
    void clearOrderHistory(int daysToKeep = 30);
};

/**
 * @brief Order execution strategies
 */
class ExecutionStrategy {
public:
    virtual ~ExecutionStrategy() = default;
    virtual bool execute(Order& order, const std::vector<ExchangeAPI*>& exchanges) = 0;
    virtual std::string getName() const = 0;
};

class TWAPStrategy : public ExecutionStrategy {
public:
    TWAPStrategy(Duration timeWindow, int slices);
    bool execute(Order& order, const std::vector<ExchangeAPI*>& exchanges) override;
    std::string getName() const override { return "TWAP"; }
    
private:
    Duration timeWindow_;
    int slices_;
};

class VWAPStrategy : public ExecutionStrategy {
public:
    VWAPStrategy(Duration timeWindow);
    bool execute(Order& order, const std::vector<ExchangeAPI*>& exchanges) override;
    std::string getName() const override { return "VWAP"; }
    
private:
    Duration timeWindow_;
};

class MinimizeSlippageStrategy : public ExecutionStrategy {
public:
    MinimizeSlippageStrategy(double maxSlippage);
    bool execute(Order& order, const std::vector<ExchangeAPI*>& exchanges) override;
    std::string getName() const override { return "MinSlippage"; }
    
private:
    double maxSlippage_;
};

/**
 * @brief Smart order router for optimal execution
 */
class SmartOrderRouter {
public:
    SmartOrderRouter();
    
    Exchange selectBestExchange(const Order& order, 
                              const std::vector<Exchange>& availableExchanges) const;
    
    std::vector<Order> splitOrder(const Order& order, int maxSlices) const;
    
    double estimateExecutionCost(const Order& order, Exchange exchange) const;
    
private:
    struct ExchangeMetrics {
        double avgSpread;
        double avgSlippage;
        double fillRate;
        Volume avgVolume;
        TimePoint lastUpdate;
    };
    
    std::unordered_map<Exchange, ExchangeMetrics> exchangeMetrics_;
    
    void updateExchangeMetrics(Exchange exchange, const Order& order, 
                             double slippage, bool filled);
};

} // namespace MasterMind

#endif // MASTERMIND_ORDER_MANAGER_H 