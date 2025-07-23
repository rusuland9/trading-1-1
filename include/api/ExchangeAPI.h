#ifndef MASTERMIND_EXCHANGE_API_H
#define MASTERMIND_EXCHANGE_API_H

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <string>
#include "core/Types.h"

namespace MasterMind {

/**
 * @brief Base class for all exchange API implementations
 * 
 * Provides a unified interface for interacting with different exchanges
 * including crypto exchanges (Binance, Deribit, Coinbase) and forex platforms (MT4/MT5)
 */
class ExchangeAPI {
public:
    /**
     * @brief Constructor
     * @param exchangeType The type of exchange
     */
    explicit ExchangeAPI(Exchange exchangeType);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~ExchangeAPI() = default;
    
    // Connection management
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual bool reconnect() = 0;
    
    // Authentication
    virtual bool authenticate(const std::string& apiKey, 
                            const std::string& apiSecret,
                            const std::string& passphrase = "") = 0;
    virtual bool isAuthenticated() const = 0;
    
    // Market data
    virtual bool subscribeMarketData(const std::vector<Symbol>& symbols) = 0;
    virtual bool unsubscribeMarketData(const std::vector<Symbol>& symbols) = 0;
    virtual Tick getLastTick(const Symbol& symbol) const = 0;
    virtual std::vector<OHLC> getHistoricalData(const Symbol& symbol, 
                                               TimePoint start, 
                                               TimePoint end,
                                               Duration interval) const = 0;
    
    // Order management
    virtual OrderId placeOrder(const Order& order) = 0;
    virtual bool cancelOrder(const OrderId& orderId) = 0;
    virtual bool modifyOrder(const OrderId& orderId, const Order& newOrder) = 0;
    virtual Order getOrder(const OrderId& orderId) const = 0;
    virtual std::vector<Order> getActiveOrders() const = 0;
    virtual std::vector<Order> getOrderHistory(const Symbol& symbol = "", int limit = 100) const = 0;
    
    // Position management
    virtual std::vector<Position> getPositions() const = 0;
    virtual Position getPosition(const Symbol& symbol) const = 0;
    virtual bool closePosition(const Symbol& symbol) = 0;
    virtual bool closeAllPositions() = 0;
    
    // Account information
    virtual AccountInfo getAccountInfo() const = 0;
    virtual double getBalance() const = 0;
    virtual double getEquity() const = 0;
    virtual double getMargin() const = 0;
    virtual double getFreeMargin() const = 0;
    
    // Instrument information
    virtual std::vector<InstrumentSpec> getInstruments() const = 0;
    virtual InstrumentSpec getInstrumentSpec(const Symbol& symbol) const = 0;
    virtual bool isSymbolAvailable(const Symbol& symbol) const = 0;
    
    // Exchange-specific information
    Exchange getExchangeType() const { return exchangeType_; }
    virtual std::string getExchangeName() const = 0;
    virtual std::vector<AssetClass> getSupportedAssetClasses() const = 0;
    
    // Callbacks for real-time updates
    void setTickCallback(TickCallback callback) { tickCallback_ = callback; }
    void setOrderCallback(OrderCallback callback) { orderCallback_ = callback; }
    void setPositionCallback(std::function<void(const Position&)> callback) { positionCallback_ = callback; }
    void setAccountCallback(std::function<void(const AccountInfo&)> callback) { accountCallback_ = callback; }
    
    // Trading session information
    virtual bool isTradingSessionOpen() const = 0;
    virtual TimePoint getNextTradingSession() const = 0;
    virtual std::vector<std::pair<TimePoint, TimePoint>> getTradingSessions() const = 0;
    
    // Fee and cost calculations
    virtual double calculateTradingFee(const Order& order) const = 0;
    virtual double calculateMarginRequirement(const Order& order) const = 0;
    
    // Error handling
    virtual std::string getLastError() const = 0;
    virtual void clearErrors() = 0;
    
protected:
    Exchange exchangeType_;
    
    // Callbacks
    TickCallback tickCallback_;
    OrderCallback orderCallback_;
    std::function<void(const Position&)> positionCallback_;
    std::function<void(const AccountInfo&)> accountCallback_;
    
    // Helper methods for derived classes
    void notifyTick(const Tick& tick);
    void notifyOrderUpdate(const Order& order);
    void notifyPositionUpdate(const Position& position);
    void notifyAccountUpdate(const AccountInfo& account);
    
    // Validation helpers
    virtual bool validateSymbol(const Symbol& symbol) const = 0;
    virtual bool validateOrder(const Order& order) const = 0;
    
    // Connection state
    mutable std::mutex connectionMutex_;
    std::atomic<bool> connected_;
    std::atomic<bool> authenticated_;
    std::string lastError_;
};

/**
 * @brief WebSocket-based exchange API for real-time data
 */
class WebSocketExchangeAPI : public ExchangeAPI {
public:
    explicit WebSocketExchangeAPI(Exchange exchangeType);
    virtual ~WebSocketExchangeAPI();
    
    // WebSocket specific methods
    virtual bool connectWebSocket() = 0;
    virtual bool disconnectWebSocket() = 0;
    virtual bool isWebSocketConnected() const = 0;
    
protected:
    // WebSocket event handlers
    virtual void onWebSocketMessage(const std::string& message) = 0;
    virtual void onWebSocketConnect() = 0;
    virtual void onWebSocketDisconnect() = 0;
    virtual void onWebSocketError(const std::string& error) = 0;
    
    std::atomic<bool> wsConnected_;
};

/**
 * @brief REST-based exchange API for standard operations
 */
class RestExchangeAPI : public ExchangeAPI {
public:
    explicit RestExchangeAPI(Exchange exchangeType);
    virtual ~RestExchangeAPI();
    
    // REST specific methods
    virtual std::string makeRequest(const std::string& endpoint, 
                                  const std::string& method,
                                  const std::string& params = "") const = 0;
    
protected:
    // HTTP helpers
    virtual std::string buildAuthHeader() const = 0;
    virtual std::string signRequest(const std::string& request) const = 0;
    
    std::string baseUrl_;
    std::string apiKey_;
    std::string apiSecret_;
    std::string passphrase_;
};

/**
 * @brief Factory for creating exchange API instances
 */
class ExchangeAPIFactory {
public:
    static std::unique_ptr<ExchangeAPI> createExchangeAPI(Exchange exchange);
    static std::unique_ptr<ExchangeAPI> createBinanceAPI();
    static std::unique_ptr<ExchangeAPI> createDeribitAPI();
    static std::unique_ptr<ExchangeAPI> createCoinbaseAPI();
    static std::unique_ptr<ExchangeAPI> createMT4API();
    static std::unique_ptr<ExchangeAPI> createMT5API();
    
    // Configuration-based factory
    static std::unique_ptr<ExchangeAPI> createFromConfig(const std::string& configFile);
};

} // namespace MasterMind

#endif // MASTERMIND_EXCHANGE_API_H 