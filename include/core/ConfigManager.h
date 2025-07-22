#ifndef MASTERMIND_CONFIG_MANAGER_H
#define MASTERMIND_CONFIG_MANAGER_H

#include "Types.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

namespace MasterMind {

/**
 * @brief Configuration management system for Master Mind trading strategy
 * 
 * Handles loading and managing configuration from JSON/YAML files including:
 * - Symbol-specific configurations and capital allocation
 * - Risk management parameters
 * - Exchange API credentials and settings
 * - Trading session configurations
 * - Pattern detection parameters
 */
class ConfigManager {
public:
    /**
     * @brief Constructor
     */
    ConfigManager();
    
    /**
     * @brief Destructor
     */
    ~ConfigManager();
    
    // Configuration loading and saving
    bool loadConfiguration(const std::string& configFile);
    bool saveConfiguration(const std::string& configFile) const;
    bool reloadConfiguration();
    bool isConfigurationValid() const;
    
    // Symbol configuration
    bool addSymbolConfig(const SymbolConfig& config);
    bool removeSymbolConfig(const Symbol& symbol);
    bool updateSymbolConfig(const SymbolConfig& config);
    SymbolConfig getSymbolConfig(const Symbol& symbol) const;
    std::vector<SymbolConfig> getAllSymbolConfigs() const;
    std::vector<Symbol> getEnabledSymbols() const;
    
    // Risk management configuration
    void setGlobalRiskParameters(const RiskParameters& params);
    RiskParameters getGlobalRiskParameters() const;
    void setSymbolRiskParameters(const Symbol& symbol, const RiskParameters& params);
    RiskParameters getSymbolRiskParameters(const Symbol& symbol) const;
    
    // Exchange configuration
    struct ExchangeConfig {
        Exchange exchange;
        std::string apiKey;
        std::string apiSecret;
        std::string passphrase;
        std::string baseUrl;
        bool enabled;
        bool testnet;
        double maxOrderSize;
        double minOrderSize;
        int rateLimitRequests;
        int rateLimitWindow;
        
        ExchangeConfig() : exchange(Exchange::BINANCE), enabled(false), testnet(true),
                          maxOrderSize(1000000), minOrderSize(0.01), 
                          rateLimitRequests(100), rateLimitWindow(60) {}
    };
    
    void setExchangeConfig(const ExchangeConfig& config);
    ExchangeConfig getExchangeConfig(Exchange exchange) const;
    std::vector<ExchangeConfig> getEnabledExchangeConfigs() const;
    
    // Trading session configuration
    void setTradingSession(const Symbol& symbol, TimePoint start, TimePoint end);
    std::pair<TimePoint, TimePoint> getTradingSession(const Symbol& symbol) const;
    void setGlobalTradingSession(TimePoint start, TimePoint end);
    std::pair<TimePoint, TimePoint> getGlobalTradingSession() const;
    
    // Pattern detection configuration
    struct PatternConfig {
        bool setup1Enabled = true;
        bool setup2Enabled = true;
        double minConfidence = 0.7;
        double partialBrickThreshold = 0.75;
        int tickBuffer = 2;
        double riskRewardRatio = 2.0;
        int maxPatternsPerSymbol = 3;
        Duration patternTimeout = std::chrono::minutes(30);
    };
    
    void setPatternConfig(const PatternConfig& config);
    PatternConfig getPatternConfig() const;
    
    // Logging configuration
    struct LogConfig {
        std::string logLevel = "INFO";
        std::string logFile = "logs/mastermind.log";
        bool enableConsoleLog = true;
        bool enableFileLog = true;
        bool enableAuditTrail = true;
        int maxLogFileSize = 100; // MB
        int maxLogFiles = 10;
        bool logMarketData = false;
        bool logOrderDetails = true;
        bool logRiskEvents = true;
    };
    
    void setLogConfig(const LogConfig& config);
    LogConfig getLogConfig() const;
    
    // Database configuration
    struct DatabaseConfig {
        std::string type = "SQLite";
        std::string connectionString = "database/mastermind.db";
        bool enableBackup = true;
        int backupInterval = 24; // hours
        bool enableEncryption = true;
        std::string encryptionKey;
    };
    
    void setDatabaseConfig(const DatabaseConfig& config);
    DatabaseConfig getDatabaseConfig() const;
    
    // General system configuration
    struct SystemConfig {
        int maxThreads = 8;
        bool enablePaperTrading = false;
        bool autoStart = false;
        Duration heartbeatInterval = std::chrono::seconds(30);
        Duration reconnectInterval = std::chrono::seconds(60);
        int maxReconnectAttempts = 5;
        bool enableWebInterface = false;
        int webPort = 8080;
    };
    
    void setSystemConfig(const SystemConfig& config);
    SystemConfig getSystemConfig() const;
    
    // Configuration validation
    std::vector<std::string> validateConfiguration() const;
    bool isSymbolConfigValid(const SymbolConfig& config) const;
    bool isRiskConfigValid(const RiskParameters& params) const;
    bool isExchangeConfigValid(const ExchangeConfig& config) const;
    
    // Configuration templates and defaults
    SymbolConfig getDefaultSymbolConfig(const Symbol& symbol, AssetClass assetClass) const;
    RiskParameters getDefaultRiskParameters() const;
    ExchangeConfig getDefaultExchangeConfig(Exchange exchange) const;
    
    // Hot reload and file watching
    void enableConfigFileWatching(bool enable);
    bool isConfigFileWatchingEnabled() const;
    void onConfigFileChanged();
    
    // Configuration export/import
    bool exportConfiguration(const std::string& filename, const std::string& format = "json") const;
    bool importConfiguration(const std::string& filename);
    bool mergeConfiguration(const std::string& filename);
    
    // Environment-based configuration
    void setEnvironment(const std::string& environment);
    std::string getEnvironment() const;
    bool loadEnvironmentConfig();
    
private:
    // Configuration storage
    std::unordered_map<Symbol, SymbolConfig> symbolConfigs_;
    RiskParameters globalRiskParams_;
    std::unordered_map<Exchange, ExchangeConfig> exchangeConfigs_;
    std::unordered_map<Symbol, std::pair<TimePoint, TimePoint>> tradingSessions_;
    std::pair<TimePoint, TimePoint> globalTradingSession_;
    PatternConfig patternConfig_;
    LogConfig logConfig_;
    DatabaseConfig databaseConfig_;
    SystemConfig systemConfig_;
    
    // State management
    std::string configFilePath_;
    std::string environment_;
    bool configValid_;
    bool fileWatchingEnabled_;
    TimePoint lastModified_;
    
    // Thread safety
    mutable std::mutex configMutex_;
    
    // File format handlers
    bool loadJsonConfig(const std::string& filename);
    bool loadYamlConfig(const std::string& filename);
    bool saveJsonConfig(const std::string& filename) const;
    bool saveYamlConfig(const std::string& filename) const;
    
    // JSON parsing helpers
    SymbolConfig parseSymbolConfig(const std::string& json) const;
    RiskParameters parseRiskParameters(const std::string& json) const;
    ExchangeConfig parseExchangeConfig(const std::string& json) const;
    
    // JSON serialization helpers
    std::string symbolConfigToJson(const SymbolConfig& config) const;
    std::string riskParametersToJson(const RiskParameters& params) const;
    std::string exchangeConfigToJson(const ExchangeConfig& config) const;
    
    // Configuration validation helpers
    bool validateSymbolConfig(const SymbolConfig& config, std::vector<std::string>& errors) const;
    bool validateRiskParameters(const RiskParameters& params, std::vector<std::string>& errors) const;
    bool validateExchangeConfig(const ExchangeConfig& config, std::vector<std::string>& errors) const;
    
    // Default value generators
    void initializeDefaults();
    void setDefaultSymbolConfigs();
    void setDefaultRiskParameters();
    void setDefaultExchangeConfigs();
    
    // Environment variable helpers
    std::string getEnvironmentVariable(const std::string& key, const std::string& defaultValue = "") const;
    void loadEnvironmentVariables();
    
    // File system helpers
    bool fileExists(const std::string& filename) const;
    TimePoint getFileModificationTime(const std::string& filename) const;
    std::string detectFileFormat(const std::string& filename) const;
    
    // Utility methods
    void logConfigChange(const std::string& change) const;
    void notifyConfigChange(const std::string& change) const;
};

/**
 * @brief Configuration factory for creating common configurations
 */
class ConfigFactory {
public:
    static std::unique_ptr<ConfigManager> createDemoConfig();
    static std::unique_ptr<ConfigManager> createLiveConfig();
    static std::unique_ptr<ConfigManager> createBacktestConfig();
    
    // Asset-specific configurations
    static SymbolConfig createForexConfig(const Symbol& symbol, double capitalAllocation);
    static SymbolConfig createCryptoConfig(const Symbol& symbol, double capitalAllocation);
    static SymbolConfig createFuturesConfig(const Symbol& symbol, double capitalAllocation);
    
    // Risk profile configurations
    static RiskParameters createConservativeRisk();
    static RiskParameters createAggressiveRisk();
    static RiskParameters createBalancedRisk();
};

} // namespace MasterMind

#endif // MASTERMIND_CONFIG_MANAGER_H 