#include "core/OrderManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace MasterMind {

OrderManager::OrderManager() 
    : running_(false), smartRoutingEnabled_(true), maxSlippagePercent_(0.01), 
      riskValidationEnabled_(true) {
    
    std::cout << "OrderManager initialized" << std::endl;
}

OrderManager::~OrderManager() {
    if (running_) {
        stop();
    }
}

bool OrderManager::initialize() {
    std::cout << "OrderManager initialization complete" << std::endl;
    return true;
}

void OrderManager::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    // Start worker threads (stub implementation)
    orderProcessingThread_ = std::thread(&OrderManager::orderProcessingWorker, this);
    statusUpdateThread_ = std::thread(&OrderManager::statusUpdateWorker, this);
    
    std::cout << "OrderManager started" << std::endl;
}

void OrderManager::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    orderCV_.notify_all();
    
    // Wait for threads to finish
    if (orderProcessingThread_.joinable()) {
        orderProcessingThread_.join();
    }
    if (statusUpdateThread_.joinable()) {
        statusUpdateThread_.join();
    }
    
    std::cout << "OrderManager stopped" << std::endl;
}

bool OrderManager::isRunning() const {
    return running_;
}

OrderId OrderManager::submitOrder(const Order& order) {
    if (!validateOrder(order)) {
        std::cout << "Order validation failed for " << order.symbol << std::endl;
        return "";
    }
    
    Order newOrder = order;
    newOrder.orderId = generateOrderId();
    newOrder.createTime = std::chrono::system_clock::now();
    newOrder.status = OrderStatus::PENDING;
    
    {
        std::lock_guard<std::mutex> lock(ordersMutex_);
        activeOrders_[newOrder.orderId] = newOrder;
        orderQueue_.push(newOrder);
    }
    
    orderCV_.notify_one();
    
    std::cout << "Order submitted: " << newOrder.orderId 
              << " for " << newOrder.symbol << std::endl;
    
    return newOrder.orderId;
}

bool OrderManager::cancelOrder(const OrderId& orderId) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        it->second.status = OrderStatus::CANCELLED;
        updateOrderStatus(orderId, OrderStatus::CANCELLED);
        
        std::cout << "Order cancelled: " << orderId << std::endl;
        return true;
    }
    
    return false;
}

bool OrderManager::modifyOrder(const OrderId& orderId, const Order& newOrder) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end() && it->second.status == OrderStatus::PENDING) {
        Order modified = it->second;
        modified.price = newOrder.price;
        modified.quantity = newOrder.quantity;
        modified.updateTime = std::chrono::system_clock::now();
        
        activeOrders_[orderId] = modified;
        
        std::cout << "Order modified: " << orderId << std::endl;
        return true;
    }
    
    return false;
}

Order OrderManager::getOrder(const OrderId& orderId) const {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        return it->second;
    }
    
    auto histIt = orderHistory_.find(orderId);
    if (histIt != orderHistory_.end()) {
        return histIt->second;
    }
    
    return Order(); // Return empty order if not found
}

std::vector<Order> OrderManager::getActiveOrders() const {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    std::vector<Order> orders;
    for (const auto& pair : activeOrders_) {
        orders.push_back(pair.second);
    }
    
    return orders;
}

std::vector<Order> OrderManager::getOrderHistory(const Symbol& symbol) const {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    std::vector<Order> orders;
    for (const auto& pair : orderHistory_) {
        if (symbol.empty() || pair.second.symbol == symbol) {
            orders.push_back(pair.second);
        }
    }
    
    return orders;
}

OrderStatus OrderManager::getOrderStatus(const OrderId& orderId) const {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        return it->second.status;
    }
    
    return OrderStatus::REJECTED; // Default if not found
}

void OrderManager::onOrderUpdate(const Order& order) {
    {
        std::lock_guard<std::mutex> lock(ordersMutex_);
        activeOrders_[order.orderId] = order;
    }
    
    notifyOrderUpdate(order);
    
    if (order.status == OrderStatus::FILLED || 
        order.status == OrderStatus::CANCELLED ||
        order.status == OrderStatus::REJECTED) {
        moveToHistory(order.orderId);
    }
}

void OrderManager::onFillUpdate(const OrderId& orderId, Volume fillQuantity, Price fillPrice) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        it->second.filledQuantity += fillQuantity;
        it->second.updateTime = std::chrono::system_clock::now();
        
        if (it->second.filledQuantity >= it->second.quantity) {
            it->second.status = OrderStatus::FILLED;
        } else {
            it->second.status = OrderStatus::PARTIALLY_FILLED;
        }
        
        // Calculate slippage
        double slippage = calculateSlippage(it->second, fillPrice);
        updateSlippageStats(it->second.symbol, slippage);
        
        std::cout << "Order fill: " << orderId 
                  << " (" << fillQuantity << " @ " << fillPrice 
                  << ", slippage: " << slippage << ")" << std::endl;
        
        if (fillCallback_) {
            fillCallback_(orderId, fillQuantity, fillPrice);
        }
    }
}

void OrderManager::onOrderRejected(const OrderId& orderId, const std::string& reason) {
    updateOrderStatus(orderId, OrderStatus::REJECTED);
    
    std::cout << "Order rejected: " << orderId << " - " << reason << std::endl;
    
    if (rejectionCallback_) {
        rejectionCallback_(orderId, reason);
    }
}

void OrderManager::setOrderCallback(OrderCallback callback) {
    orderCallback_ = callback;
}

void OrderManager::setFillCallback(std::function<void(const OrderId&, Volume, Price)> callback) {
    fillCallback_ = callback;
}

void OrderManager::setRejectionCallback(std::function<void(const OrderId&, const std::string&)> callback) {
    rejectionCallback_ = callback;
}

void OrderManager::setRiskValidationCallback(std::function<bool(const Order&)> callback) {
    riskValidationCallback_ = callback;
}

void OrderManager::enableRiskValidation(bool enable) {
    riskValidationEnabled_ = enable;
}

double OrderManager::getAverageSlippage(const Symbol& symbol) const {
    auto it = executionStats_.find(symbol);
    if (it != executionStats_.end() && it->second.totalOrders > 0) {
        return it->second.totalSlippage / it->second.totalOrders;
    }
    return 0.0;
}

double OrderManager::getFillRate() const {
    int totalOrders = 0;
    int filledOrders = 0;
    
    for (const auto& pair : executionStats_) {
        totalOrders += pair.second.totalOrders;
        filledOrders += pair.second.filledOrders;
    }
    
    return (totalOrders > 0) ? static_cast<double>(filledOrders) / totalOrders : 0.0;
}

int OrderManager::getActiveOrderCount() const {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    return static_cast<int>(activeOrders_.size());
}

std::vector<std::string> OrderManager::getExecutionReport() const {
    std::vector<std::string> report;
    
    report.push_back("=== Order Execution Report ===");
    report.push_back("Active Orders: " + std::to_string(getActiveOrderCount()));
    report.push_back("Fill Rate: " + std::to_string(getFillRate() * 100) + "%");
    
    for (const auto& pair : executionStats_) {
        const auto& stats = pair.second;
        std::string line = pair.first + ": " + 
            std::to_string(stats.filledOrders) + "/" + std::to_string(stats.totalOrders) +
            " (avg slippage: " + std::to_string(getAverageSlippage(pair.first)) + ")";
        report.push_back(line);
    }
    
    return report;
}

// Private methods
void OrderManager::orderProcessingWorker() {
    std::unique_lock<std::mutex> lock(ordersMutex_);
    
    while (running_) {
        orderCV_.wait(lock, [this] { return !orderQueue_.empty() || !running_; });
        
        while (!orderQueue_.empty() && running_) {
            Order order = orderQueue_.front();
            orderQueue_.pop();
            
            lock.unlock();
            
            // Process order (stub implementation)
            processOrder(order);
            
            lock.lock();
        }
    }
}

void OrderManager::statusUpdateWorker() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Update order statuses (stub implementation)
        cleanupExpiredOrders();
    }
}

void OrderManager::processOrder(const Order& order) {
    // Stub implementation - simulate order processing
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    Order updatedOrder = order;
    updatedOrder.status = OrderStatus::SUBMITTED;
    updatedOrder.updateTime = std::chrono::system_clock::now();
    
    onOrderUpdate(updatedOrder);
    
    // Simulate execution after some time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Simulate fill
    onFillUpdate(order.orderId, order.quantity, order.price);
}

bool OrderManager::validateOrder(const Order& order) const {
    // Basic validation
    if (order.symbol.empty() || order.quantity <= 0 || order.price <= 0) {
        return false;
    }
    
    // Risk validation if enabled
    if (riskValidationEnabled_ && riskValidationCallback_) {
        return riskValidationCallback_(order);
    }
    
    return true;
}

OrderId OrderManager::generateOrderId() const {
    static int counter = 0;
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::stringstream ss;
    ss << "MM" << timestamp << "-" << std::setfill('0') << std::setw(4) << (++counter);
    return ss.str();
}

void OrderManager::updateOrderStatus(const OrderId& orderId, OrderStatus status) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        it->second.status = status;
        it->second.updateTime = std::chrono::system_clock::now();
    }
}

void OrderManager::moveToHistory(const OrderId& orderId) {
    std::lock_guard<std::mutex> lock(ordersMutex_);
    
    auto it = activeOrders_.find(orderId);
    if (it != activeOrders_.end()) {
        orderHistory_[orderId] = it->second;
        activeOrders_.erase(it);
    }
}

void OrderManager::cleanupExpiredOrders() {
    // Stub implementation
}

void OrderManager::notifyOrderUpdate(const Order& order) {
    if (orderCallback_) {
        orderCallback_(order);
    }
}

double OrderManager::calculateSlippage(const Order& order, Price fillPrice) const {
    return std::abs(fillPrice - order.price) / order.price;
}

void OrderManager::updateSlippageStats(const Symbol& symbol, double slippage) {
    auto& stats = executionStats_[symbol];
    stats.totalSlippage += slippage;
    stats.totalOrders++;
    stats.filledOrders++;
    stats.slippageHistory.push_back(slippage);
    
    // Keep only last 100 slippage records
    if (stats.slippageHistory.size() > 100) {
        stats.slippageHistory.erase(stats.slippageHistory.begin());
    }
}

// Stub implementations for other methods
OrderId OrderManager::submitHybridOrder(const Order& order, Volume icebergQuantity, Price pegOffset) {
    std::cout << "Hybrid order submitted (stub)" << std::endl;
    return submitOrder(order);
}

OrderId OrderManager::submitStopOrder(const Order& order, Price triggerPrice, int tickBuffer) {
    std::cout << "Stop order submitted (stub)" << std::endl;
    return submitOrder(order);
}

OrderId OrderManager::submitTrailingStop(const Order& order, Price trailAmount, bool usePercent) {
    std::cout << "Trailing stop submitted (stub)" << std::endl;
    return submitOrder(order);
}

bool OrderManager::setStopLoss(const Symbol& symbol, Price stopPrice) { 
    std::cout << "Stop loss set for " << symbol << " at " << stopPrice << std::endl;
    return true; 
}

bool OrderManager::setTakeProfit(const Symbol& symbol, Price targetPrice) { 
    std::cout << "Take profit set for " << symbol << " at " << targetPrice << std::endl;
    return true; 
}

bool OrderManager::updateTrailingStop(const Symbol& symbol, Price newTrailPrice) { 
    std::cout << "Trailing stop updated for " << symbol << std::endl;
    return true; 
}

void OrderManager::addExchange(Exchange exchange, std::unique_ptr<ExchangeAPI> api) {
    std::cout << "Exchange added (stub implementation)" << std::endl;
}

Exchange OrderManager::getBestExchange(const Symbol& symbol, OrderSide side, Volume quantity) const {
    return Exchange::BINANCE; // Default
}

bool OrderManager::routeOrder(Order& order) { 
    std::cout << "Order routed (stub)" << std::endl;
    return true; 
}

void OrderManager::setExecutionStrategy(const Symbol& symbol, const std::string& strategy) {
    executionStrategies_[symbol] = strategy;
}

void OrderManager::enableSmartRouting(bool enable) {
    smartRoutingEnabled_ = enable;
}

void OrderManager::setSlippageThreshold(double maxSlippagePercent) {
    maxSlippagePercent_ = maxSlippagePercent;
}

} // namespace MasterMind 