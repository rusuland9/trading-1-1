#ifndef MASTERMIND_BINANCE_API_H
#define MASTERMIND_BINANCE_API_H

#include "api/ExchangeAPI.h"
#include <string>
#include <vector>

namespace MasterMind {

/**
 * @brief Binance exchange API implementation
 * 
 * Provides integration with Binance spot and futures trading
 * Supports both testnet and live trading environments
 */
class BinanceAPI : public RestExchangeAPI {
public:
    /**
     * @brief Constructor
     */
    BinanceAPI();
    
    /**
     * @brief Destructor
     */
    virtual ~BinanceAPI();
    
    // Connection management
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    bool reconnect() override;
    
    // Authentication
    bool authenticate(const std::string& apiKey, 
                     const std::string& apiSecret,
                     const std::string& passphrase = "") override;
    bool isAuthenticated() const override;
    
    // Market data
    bool subscribeMarketData(const std::vector<Symbol>& symbols) override;
    bool unsubscribeMarketData(const std::vector<Symbol>& symbols) override;
    Tick getLastTick(const Symbol& symbol) const override;
    std::vector<OHLC> getHistoricalData(const Symbol& symbol, 
                                       TimePoint start, 
                                       TimePoint end,
                                       Duration interval) const override;
    
    // Order management
    OrderId placeOrder(const Order& order) override;
    bool cancelOrder(const OrderId& orderId) override;
    bool modifyOrder(const OrderId& orderId, const Order& newOrder) override;
    Order getOrder(const OrderId& orderId) const override;
    std::vector<Order> getActiveOrders() const override;
    std::vector<Order> getOrderHistory(const Symbol& symbol = "", int limit = 100) const override;
    
    // Position management
    std::vector<Position> getPositions() const override;
    Position getPosition(const Symbol& symbol) const override;
    bool closePosition(const Symbol& symbol) override;
    bool closeAllPositions() override;
    
    // Account information
    AccountInfo getAccountInfo() const override;
    double getBalance() const override;
    double getEquity() const override;
    double getMargin() const override;
    double getFreeMargin() const override;
    
    // Instrument information
    std::vector<InstrumentSpec> getInstruments() const override;
    InstrumentSpec getInstrumentSpec(const Symbol& symbol) const override;
    bool isSymbolAvailable(const Symbol& symbol) const override;
    
    // Exchange-specific information
    std::string getExchangeName() const override;
    std::vector<AssetClass> getSupportedAssetClasses() const override;
    
    // Trading session information
    bool isTradingSessionOpen() const override;
    TimePoint getNextTradingSession() const override;
    std::vector<std::pair<TimePoint, TimePoint>> getTradingSessions() const override;
    
    // Fee and cost calculations
    double calculateTradingFee(const Order& order) const override;
    double calculateMarginRequirement(const Order& order) const override;
    
    // Error handling
    std::string getLastError() const override;
    void clearErrors() override;

protected:
    // Validation helpers
    bool validateSymbol(const Symbol& symbol) const override;
    bool validateOrder(const Order& order) const override;
    
    // HTTP request methods
    std::string makeRequest(const std::string& endpoint, 
                           const std::string& method,
                           const std::string& params = "") const override;
    
    // Authentication helpers
    std::string makeAuthenticatedRequest(const std::string& endpoint,
                                        const std::string& method,
                                        const std::string& params = "") const;
    std::string buildAuthHeader() const override;
    std::string signRequest(const std::string& request) const override;
    
    // Utility methods
    std::string getOrderTypeString(OrderType type) const;
    std::string generateOrderId() const;
};

} // namespace MasterMind

#endif // MASTERMIND_BINANCE_API_H 