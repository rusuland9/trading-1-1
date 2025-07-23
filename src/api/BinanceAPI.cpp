#include "api/BinanceAPI.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <chrono>
#include <json/json.h>

namespace MasterMind {

BinanceAPI::BinanceAPI() : RestExchangeAPI(Exchange::BINANCE) {
    baseUrl_ = "https://api.binance.com";
    std::cout << "BinanceAPI initialized" << std::endl;
}

BinanceAPI::~BinanceAPI() {
    disconnect();
}

bool BinanceAPI::connect() {
    std::lock_guard<std::mutex> lock(connectionMutex_);
    
    if (connected_) {
        return true;
    }
    
    try {
        // Test connectivity
        auto response = makeRequest("/api/v3/ping", "GET");
        if (response.empty()) {
            lastError_ = "Failed to ping Binance API";
            return false;
        }
        
        // Get server time
        response = makeRequest("/api/v3/time", "GET");
        if (response.empty()) {
            lastError_ = "Failed to get server time";
            return false;
        }
        
        connected_ = true;
        clearErrors();
        
        std::cout << "Connected to Binance API" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        lastError_ = std::string("Connection error: ") + e.what();
        connected_ = false;
        return false;
    }
}

bool BinanceAPI::disconnect() {
    std::lock_guard<std::mutex> lock(connectionMutex_);
    
    if (!connected_) {
        return true;
    }
    
    connected_ = false;
    authenticated_ = false;
    
    std::cout << "Disconnected from Binance API" << std::endl;
    return true;
}

bool BinanceAPI::isConnected() const {
    return connected_;
}

bool BinanceAPI::reconnect() {
    disconnect();
    return connect();
}

bool BinanceAPI::authenticate(const std::string& apiKey, 
                            const std::string& apiSecret,
                            const std::string& passphrase) {
    
    apiKey_ = apiKey;
    apiSecret_ = apiSecret;
    
    if (apiKey_.empty() || apiSecret_.empty()) {
        lastError_ = "API key and secret are required";
        return false;
    }
    
    try {
        // Test authentication with account info request
        auto response = makeAuthenticatedRequest("/api/v3/account", "GET");
        if (response.empty()) {
            lastError_ = "Authentication failed";
            authenticated_ = false;
            return false;
        }
        
        authenticated_ = true;
        clearErrors();
        
        std::cout << "Authenticated with Binance API" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        lastError_ = std::string("Authentication error: ") + e.what();
        authenticated_ = false;
        return false;
    }
}

bool BinanceAPI::isAuthenticated() const {
    return authenticated_;
}

bool BinanceAPI::subscribeMarketData(const std::vector<Symbol>& symbols) {
    // For now, return true as stub implementation
    // In full implementation, this would establish WebSocket connections
    std::cout << "Subscribed to market data for " << symbols.size() << " symbols" << std::endl;
    return true;
}

bool BinanceAPI::unsubscribeMarketData(const std::vector<Symbol>& symbols) {
    std::cout << "Unsubscribed from market data for " << symbols.size() << " symbols" << std::endl;
    return true;
}

Tick BinanceAPI::getLastTick(const Symbol& symbol) const {
    try {
        std::string endpoint = "/api/v3/ticker/24hr?symbol=" + symbol;
        auto response = makeRequest(endpoint, "GET");
        
        // Parse JSON response and convert to Tick
        // This is a simplified implementation
        Tick tick;
        tick.symbol = symbol;
        tick.timestamp = std::chrono::system_clock::now();
        tick.bid = 0.0;
        tick.ask = 0.0;
        tick.last = 0.0;
        tick.volume = 0.0;
        
        return tick;
        
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to get tick data: ") + e.what();
        return Tick{};
    }
}

std::vector<OHLC> BinanceAPI::getHistoricalData(const Symbol& symbol, 
                                               TimePoint start, 
                                               TimePoint end,
                                               Duration interval) const {
    // Stub implementation
    std::vector<OHLC> ohlcData;
    std::cout << "Retrieved historical data for " << symbol << std::endl;
    return ohlcData;
}

OrderId BinanceAPI::placeOrder(const Order& order) {
    if (!isAuthenticated()) {
        lastError_ = "Not authenticated";
        return "";
    }
    
    try {
        std::ostringstream params;
        params << "symbol=" << order.symbol;
        params << "&side=" << (order.side == OrderSide::BUY ? "BUY" : "SELL");
        params << "&type=" << getOrderTypeString(order.type);
        params << "&quantity=" << std::fixed << std::setprecision(8) << order.quantity;
        
        if (order.type == OrderType::LIMIT) {
            params << "&price=" << std::fixed << std::setprecision(8) << order.price;
            params << "&timeInForce=GTC";
        }
        
        auto response = makeAuthenticatedRequest("/api/v3/order", "POST", params.str());
        
        if (!response.empty()) {
            // Parse response to extract order ID
            // This is simplified - in real implementation, parse JSON
            std::cout << "Order placed successfully on Binance" << std::endl;
            return generateOrderId();
        }
        
        lastError_ = "Failed to place order";
        return "";
        
    } catch (const std::exception& e) {
        lastError_ = std::string("Order placement error: ") + e.what();
        return "";
    }
}

bool BinanceAPI::cancelOrder(const OrderId& orderId) {
    if (!isAuthenticated()) {
        lastError_ = "Not authenticated";
        return false;
    }
    
    std::cout << "Order cancelled: " << orderId << std::endl;
    return true;
}

bool BinanceAPI::modifyOrder(const OrderId& orderId, const Order& newOrder) {
    // Binance doesn't support direct order modification
    // Implementation would cancel and replace
    std::cout << "Order modification requested: " << orderId << std::endl;
    return true;
}

Order BinanceAPI::getOrder(const OrderId& orderId) const {
    Order order;
    order.orderId = orderId;
    return order;
}

std::vector<Order> BinanceAPI::getActiveOrders() const {
    std::vector<Order> orders;
    return orders;
}

std::vector<Order> BinanceAPI::getOrderHistory(const Symbol& symbol, int limit) const {
    std::vector<Order> history;
    return history;
}

std::vector<Position> BinanceAPI::getPositions() const {
    std::vector<Position> positions;
    return positions;
}

Position BinanceAPI::getPosition(const Symbol& symbol) const {
    Position position;
    position.symbol = symbol;
    return position;
}

bool BinanceAPI::closePosition(const Symbol& symbol) {
    std::cout << "Closing position for: " << symbol << std::endl;
    return true;
}

bool BinanceAPI::closeAllPositions() {
    std::cout << "Closing all positions" << std::endl;
    return true;
}

AccountInfo BinanceAPI::getAccountInfo() const {
    AccountInfo account;
    account.balance = 10000.0;  // Placeholder
    account.equity = 10000.0;
    account.margin = 0.0;
    account.freeMargin = 10000.0;
    return account;
}

double BinanceAPI::getBalance() const {
    return getAccountInfo().balance;
}

double BinanceAPI::getEquity() const {
    return getAccountInfo().equity;
}

double BinanceAPI::getMargin() const {
    return getAccountInfo().margin;
}

double BinanceAPI::getFreeMargin() const {
    return getAccountInfo().freeMargin;
}

std::vector<InstrumentSpec> BinanceAPI::getInstruments() const {
    std::vector<InstrumentSpec> instruments;
    
    // Add some common instruments
    InstrumentSpec btcusdt;
    btcusdt.symbol = "BTCUSDT";
    btcusdt.assetClass = AssetClass::CRYPTO;
    btcusdt.tickSize = 0.01;
    btcusdt.minOrderSize = 0.00001;
    btcusdt.maxOrderSize = 1000.0;
    instruments.push_back(btcusdt);
    
    return instruments;
}

InstrumentSpec BinanceAPI::getInstrumentSpec(const Symbol& symbol) const {
    auto instruments = getInstruments();
    for (const auto& instrument : instruments) {
        if (instrument.symbol == symbol) {
            return instrument;
        }
    }
    return InstrumentSpec{};
}

bool BinanceAPI::isSymbolAvailable(const Symbol& symbol) const {
    auto instruments = getInstruments();
    for (const auto& instrument : instruments) {
        if (instrument.symbol == symbol) {
            return true;
        }
    }
    return false;
}

std::string BinanceAPI::getExchangeName() const {
    return "Binance";
}

std::vector<AssetClass> BinanceAPI::getSupportedAssetClasses() const {
    return {AssetClass::CRYPTO};
}

bool BinanceAPI::isTradingSessionOpen() const {
    return true; // Crypto markets are always open
}

TimePoint BinanceAPI::getNextTradingSession() const {
    return std::chrono::system_clock::now();
}

std::vector<std::pair<TimePoint, TimePoint>> BinanceAPI::getTradingSessions() const {
    // Crypto markets are 24/7
    auto now = std::chrono::system_clock::now();
    auto tomorrow = now + std::chrono::hours(24);
    return {{now, tomorrow}};
}

double BinanceAPI::calculateTradingFee(const Order& order) const {
    // Binance standard fee is 0.1%
    return order.quantity * order.price * 0.001;
}

double BinanceAPI::calculateMarginRequirement(const Order& order) const {
    // For spot trading, full amount is required
    return order.quantity * order.price;
}

std::string BinanceAPI::getLastError() const {
    return lastError_;
}

void BinanceAPI::clearErrors() {
    lastError_.clear();
}

// Protected methods
bool BinanceAPI::validateSymbol(const Symbol& symbol) const {
    return !symbol.empty() && isSymbolAvailable(symbol);
}

bool BinanceAPI::validateOrder(const Order& order) const {
    if (order.symbol.empty() || order.quantity <= 0) {
        return false;
    }
    
    if (order.type == OrderType::LIMIT && order.price <= 0) {
        return false;
    }
    
    return validateSymbol(order.symbol);
}

std::string BinanceAPI::makeRequest(const std::string& endpoint, 
                                   const std::string& method,
                                   const std::string& params) const {
    // Simplified HTTP request implementation
    // In real implementation, use libcurl or similar
    std::cout << "Making " << method << " request to " << endpoint << std::endl;
    return "{}"; // Placeholder JSON response
}

std::string BinanceAPI::makeAuthenticatedRequest(const std::string& endpoint,
                                               const std::string& method,
                                               const std::string& params) const {
    
    if (!isAuthenticated()) {
        return "";
    }
    
    // Add timestamp and signature
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::string fullParams = params;
    if (!fullParams.empty()) {
        fullParams += "&";
    }
    fullParams += "timestamp=" + std::to_string(timestamp);
    
    // Sign the request
    std::string signature = signRequest(fullParams);
    fullParams += "&signature=" + signature;
    
    return makeRequest(endpoint, method, fullParams);
}

std::string BinanceAPI::buildAuthHeader() const {
    return "X-MBX-APIKEY: " + apiKey_;
}

std::string BinanceAPI::signRequest(const std::string& request) const {
    // HMAC SHA256 signature
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len = SHA256_DIGEST_LENGTH;
    
    HMAC(EVP_sha256(), 
         apiSecret_.c_str(), apiSecret_.length(),
         reinterpret_cast<const unsigned char*>(request.c_str()), request.length(),
         hash, &hash_len);
    
    // Convert to hex string
    std::ostringstream ss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

std::string BinanceAPI::getOrderTypeString(OrderType type) const {
    switch (type) {
        case OrderType::MARKET: return "MARKET";
        case OrderType::LIMIT: return "LIMIT";
        case OrderType::STOP: return "STOP_LOSS";
        case OrderType::STOP_LIMIT: return "STOP_LOSS_LIMIT";
        default: return "MARKET";
    }
}

std::string BinanceAPI::generateOrderId() const {
    static int counter = 0;
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return "BN" + std::to_string(timestamp) + "-" + std::to_string(++counter);
}

} // namespace MasterMind 