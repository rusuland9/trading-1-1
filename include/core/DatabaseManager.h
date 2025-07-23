#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace MasterMind {

/**
 * @brief Database management system for Master Mind trading system
 * 
 * Handles persistent storage of:
 * - Trading history and audit trails
 * - Configuration backups
 * - Performance statistics
 * - Risk management records
 */
class DatabaseManager {
public:
    /**
     * @brief Constructor
     */
    DatabaseManager();
    
    /**
     * @brief Destructor
     */
    ~DatabaseManager();
    
    // Database lifecycle
    bool initialize(const std::string& connectionString);
    bool connect();
    bool disconnect();
    bool isConnected() const;
    
    // Schema management
    bool createTables();
    bool dropTables();
    bool migrateTables();
    bool validateSchema() const;
    
    // Order and trade storage
    bool insertOrder(const Order& order);
    bool updateOrder(const Order& order);
    bool deleteOrder(const OrderId& orderId);
    Order getOrder(const OrderId& orderId) const;
    std::vector<Order> getOrderHistory(const Symbol& symbol = "", int limit = 1000) const;
    
    // Position tracking
    bool insertPosition(const Position& position);
    bool updatePosition(const Position& position);
    std::vector<Position> getPositions(const Symbol& symbol = "") const;
    
    // Performance statistics
    bool insertTradeResult(const OrderId& orderId, double pnl, const std::string& strategy);
    bool updatePerformanceStats(const TradingStats& stats);
    TradingStats getPerformanceStats() const;
    
    // Risk management data
    bool insertRiskEvent(const std::string& event, const std::string& details);
    bool insertCounterResult(int counterNumber, double pnl, int orderCount);
    std::vector<std::string> getRiskEvents(int limit = 100) const;
    
    // Configuration backup
    bool backupConfiguration(const std::string& configJson);
    std::string getLatestConfiguration() const;
    std::vector<std::string> getConfigurationHistory() const;
    
    // Audit trail
    bool insertAuditEntry(const std::string& action, const std::string& details, 
                         const std::string& userId = "system");
    std::vector<std::string> getAuditTrail(const TimePoint& startTime, 
                                          const TimePoint& endTime) const;
    
    // Data maintenance
    bool cleanupOldData(int daysToKeep = 90);
    bool vacuum();
    bool backup(const std::string& backupPath);
    bool restore(const std::string& backupPath);
    
    // Query helpers
    int getOrderCount(const Symbol& symbol = "") const;
    double getTotalPnL(const Symbol& symbol = "") const;
    int getTradeCount() const;
    double getWinRate() const;
    
    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // Error handling
    std::string getLastError() const;
    bool hasError() const;
    void clearError();

private:
    // Database connection
    std::string connectionString_;
    bool connected_;
    std::string lastError_;
    
    // Thread safety
    mutable std::mutex dbMutex_;
    
    // Internal implementation details
    void* dbHandle_; // SQLite database handle
    
    // Schema creation helpers
    bool createOrdersTable();
    bool createPositionsTable();
    bool createPerformanceTable();
    bool createRiskEventsTable();
    bool createAuditTrailTable();
    bool createConfigBackupTable();
    
    // Query execution helpers
    bool executeQuery(const std::string& query) const;
    std::vector<std::vector<std::string>> executeSelect(const std::string& query) const;
    
    // Data conversion helpers
    std::string orderToJson(const Order& order) const;
    Order jsonToOrder(const std::string& json) const;
    std::string positionToJson(const Position& position) const;
    Position jsonToPosition(const std::string& json) const;
    
    // Utility methods
    std::string getCurrentTimestamp() const;
    bool tableExists(const std::string& tableName) const;
    void logError(const std::string& error);
};

} // namespace MasterMind 