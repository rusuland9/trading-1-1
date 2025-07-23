#include <QtTest/QtTest>
#include <QtCore/QDebug>
#include <memory>

#include "core/TradingEngine.h"
#include "core/RenkoChart.h"
#include "core/PatternDetector.h"
#include "core/RiskManager.h"
#include "core/OrderManager.h"
#include "core/ConfigManager.h"
#include "api/BinanceAPI.h"
#include "api/ExchangeAPI.h"

using namespace MasterMind;

/**
 * @brief Comprehensive system tests for Master Mind Trading System
 * 
 * Tests all core functionality specified in the requirements:
 * - Renko chart formation and pattern detection
 * - Risk management and counter system
 * - Order management and execution
 * - Configuration loading
 * - Exchange API integration
 */
class SystemTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Core functionality tests
    void testTradingEngineInitialization();
    void testConfigurationLoading();
    void testRenkoChartFormation();
    void testSetup1PatternDetection();
    void testSetup2PatternDetection();
    void testRiskManagement();
    void testCounterSystem();
    void testOrderManagement();
    void testExchangeAPIIntegration();
    void testPaperTradingMode();
    void testEmergencyStop();
    
    // Integration tests
    void testFullTradingWorkflow();
    void testRiskLimitEnforcement();
    void testPatternToOrderFlow();

private:
    std::unique_ptr<TradingEngine> tradingEngine_;
    std::unique_ptr<ConfigManager> configManager_;
    std::unique_ptr<RenkoChart> renkoChart_;
    std::unique_ptr<PatternDetector> patternDetector_;
    std::unique_ptr<RiskManager> riskManager_;
    std::unique_ptr<OrderManager> orderManager_;
    std::unique_ptr<BinanceAPI> binanceAPI_;
    
    void setupTestData();
    void verifyRenkoBrick(const RenkoBrick& brick, double expectedOpen, double expectedClose, bool expectedIsUp);
};

void SystemTest::initTestCase() {
    qDebug() << "Starting Master Mind Trading System Tests";
    
    // Initialize test configuration
    configManager_ = std::make_unique<ConfigManager>();
    
    // Create test components
    renkoChart_ = std::make_unique<RenkoChart>("EURUSD", 0.001);
    patternDetector_ = std::make_unique<PatternDetector>();
    riskManager_ = std::make_unique<RiskManager>();
    orderManager_ = std::make_unique<OrderManager>();
    binanceAPI_ = std::make_unique<BinanceAPI>();
    
    setupTestData();
}

void SystemTest::cleanupTestCase() {
    qDebug() << "Master Mind Trading System Tests Complete";
}

void SystemTest::testTradingEngineInitialization() {
    qDebug() << "Testing TradingEngine initialization...";
    
    // Test engine creation and initialization
    tradingEngine_ = std::make_unique<TradingEngine>("config/test_config.json");
    QVERIFY(tradingEngine_ != nullptr);
    
    // Test initialization
    bool initialized = tradingEngine_->initialize();
    QVERIFY(initialized);
    
    // Test initial state
    QVERIFY(!tradingEngine_->isRunning());
    QVERIFY(tradingEngine_->isPaperMode()); // Should start in paper mode
    QCOMPARE(tradingEngine_->getRiskStatus(), RiskStatus::NORMAL);
    
    qDebug() << "✓ TradingEngine initialization test passed";
}

void SystemTest::testConfigurationLoading() {
    qDebug() << "Testing configuration loading...";
    
    // Test configuration manager
    QVERIFY(configManager_ != nullptr);
    
    // Test loading configuration (stub test)
    bool loaded = configManager_->loadConfiguration("config/test_config.json");
    QVERIFY(loaded);
    
    // Test getting enabled symbols
    auto symbols = configManager_->getEnabledSymbols();
    QVERIFY(!symbols.empty());
    
    // Test risk parameters
    auto riskParams = configManager_->getGlobalRiskParameters();
    QVERIFY(riskParams.dailyRiskPercent > 0);
    QVERIFY(riskParams.consecutiveLossLimit > 0);
    QVERIFY(riskParams.ordersPerCounter > 0);
    
    qDebug() << "✓ Configuration loading test passed";
}

void SystemTest::testRenkoChartFormation() {
    qDebug() << "Testing Renko chart formation...";
    
    QVERIFY(renkoChart_ != nullptr);
    QCOMPARE(renkoChart_->getBrickSize(), 0.001);
    QCOMPARE(renkoChart_->getBrickCount(), static_cast<size_t>(0));
    
    // Test adding price data to form bricks
    auto now = std::chrono::system_clock::now();
    
    // Add initial price
    renkoChart_->addPrice(1.1000, now);
    
    // Add price that should form an up brick
    renkoChart_->addPrice(1.1010, now + std::chrono::seconds(10));
    
    // Verify brick formation
    QVERIFY(renkoChart_->getBrickCount() >= 1);
    
    auto lastBrick = renkoChart_->getLastBrick();
    QVERIFY(lastBrick.isUp);
    QCOMPARE(lastBrick.open, 1.1000);
    QCOMPARE(lastBrick.close, 1.1010);
    
    qDebug() << "✓ Renko chart formation test passed";
}

void SystemTest::testSetup1PatternDetection() {
    qDebug() << "Testing Setup 1 pattern detection...";
    
    QVERIFY(patternDetector_ != nullptr);
    
    // Create a RenkoChart with Setup 1 pattern (two consecutive down bricks + partial up)
    RenkoChart testChart("EURUSD", 0.001);
    auto now = std::chrono::system_clock::now();
    
    // Form down bricks
    testChart.addPrice(1.1000, now);
    testChart.addPrice(1.0990, now + std::chrono::seconds(10)); // First down brick
    testChart.addPrice(1.0980, now + std::chrono::seconds(20)); // Second down brick
    testChart.addPrice(1.0987, now + std::chrono::seconds(30)); // Partial up brick (70% completion)
    
    // Test pattern detection
    auto patterns = patternDetector_->detectPatterns(testChart);
    
    // Should detect Setup 1 pattern
    bool foundSetup1 = false;
    for (const auto& pattern : patterns) {
        if (pattern.type == PatternType::SETUP_1_CONSECUTIVE) {
            foundSetup1 = true;
            QVERIFY(pattern.confidence > 0.5);
            QCOMPARE(pattern.suggestedSide, OrderSide::BUY);
            QVERIFY(pattern.suggestedEntry > 0);
            QVERIFY(pattern.suggestedStop > 0);
            break;
        }
    }
    
    QVERIFY(foundSetup1);
    qDebug() << "✓ Setup 1 pattern detection test passed";
}

void SystemTest::testSetup2PatternDetection() {
    qDebug() << "Testing Setup 2 pattern detection...";
    
    // Create a RenkoChart with Setup 2 pattern (Green-Red-Green)
    RenkoChart testChart("EURUSD", 0.001);
    auto now = std::chrono::system_clock::now();
    
    // Form Green-Red-Green pattern
    testChart.addPrice(1.1000, now);
    testChart.addPrice(1.1010, now + std::chrono::seconds(10)); // Green brick
    testChart.addPrice(1.1000, now + std::chrono::seconds(20)); // Red brick
    testChart.addPrice(1.1010, now + std::chrono::seconds(30)); // Green brick
    testChart.addPrice(1.1018, now + std::chrono::seconds(40)); // Partial green brick (80% completion)
    
    // Test pattern detection
    auto patterns = patternDetector_->detectPatterns(testChart);
    
    // Should detect Setup 2 pattern
    bool foundSetup2 = false;
    for (const auto& pattern : patterns) {
        if (pattern.type == PatternType::SETUP_2_GREEN_RED_GREEN) {
            foundSetup2 = true;
            QVERIFY(pattern.confidence > 0.5);
            QCOMPARE(pattern.suggestedSide, OrderSide::BUY);
            QVERIFY(pattern.suggestedEntry > 0);
            QVERIFY(pattern.suggestedStop > 0);
        }
    }
    
    QVERIFY(foundSetup2);
    qDebug() << "✓ Setup 2 pattern detection test passed";
}

void SystemTest::testRiskManagement() {
    qDebug() << "Testing risk management...";
    
    QVERIFY(riskManager_ != nullptr);
    
    // Test initialization with parameters
    RiskParameters params;
    params.dailyRiskPercent = 0.02;
    params.maxDrawdownPercent = 0.05;
    params.consecutiveLossLimit = 2;
    params.ordersPerCounter = 10;
    params.paperTradingMode = true;
    
    bool initialized = riskManager_->initialize(params);
    QVERIFY(initialized);
    
    // Test risk status
    QCOMPARE(riskManager_->getCurrentRiskStatus(), RiskStatus::PAPER_MODE);
    QVERIFY(riskManager_->isPaperMode());
    
    // Test consecutive loss tracking
    Order testOrder;
    testOrder.symbol = "EURUSD";
    testOrder.quantity = 0.01;
    
    // Record consecutive losses
    riskManager_->recordTrade(testOrder, false); // Loss 1
    QCOMPARE(riskManager_->getConsecutiveLosses(), 1);
    
    riskManager_->recordTrade(testOrder, false); // Loss 2
    QCOMPARE(riskManager_->getConsecutiveLosses(), 2);
    
    // Should trigger paper mode switch
    QVERIFY(riskManager_->shouldSwitchToPaperMode());
    
    qDebug() << "✓ Risk management test passed";
}

void SystemTest::testCounterSystem() {
    qDebug() << "Testing counter system...";
    
    // Test counter management
    riskManager_->startNewCounter();
    QVERIFY(!riskManager_->isCounterComplete());
    QCOMPARE(riskManager_->getOrdersInCurrentCounter(), 0);
    
    // Add orders to counter
    for (int i = 0; i < 5; i++) {
        Order order;
        order.orderId = "TEST-" + std::to_string(i);
        order.symbol = "EURUSD";
        order.quantity = 0.01;
        riskManager_->addOrderToCounter(order);
    }
    
    QCOMPARE(riskManager_->getOrdersInCurrentCounter(), 5);
    QVERIFY(!riskManager_->isCounterComplete()); // Should complete at 10 orders
    
    qDebug() << "✓ Counter system test passed";
}

void SystemTest::testOrderManagement() {
    qDebug() << "Testing order management...";
    
    QVERIFY(orderManager_ != nullptr);
    
    // Initialize order manager
    bool initialized = orderManager_->initialize();
    QVERIFY(initialized);
    
    // Test order submission
    Order testOrder;
    testOrder.symbol = "EURUSD";
    testOrder.side = OrderSide::BUY;
    testOrder.type = OrderType::LIMIT;
    testOrder.quantity = 0.01;
    testOrder.price = 1.1000;
    
    OrderId orderId = orderManager_->submitOrder(testOrder);
    QVERIFY(!orderId.empty());
    
    // Test getting active orders
    auto activeOrders = orderManager_->getActiveOrders();
    QVERIFY(!activeOrders.empty());
    
    // Test order cancellation
    bool cancelled = orderManager_->cancelOrder(orderId);
    QVERIFY(cancelled);
    
    qDebug() << "✓ Order management test passed";
}

void SystemTest::testExchangeAPIIntegration() {
    qDebug() << "Testing exchange API integration...";
    
    QVERIFY(binanceAPI_ != nullptr);
    QCOMPARE(binanceAPI_->getExchangeName(), std::string("Binance"));
    
    // Test connection (stub)
    bool connected = binanceAPI_->connect();
    QVERIFY(connected);
    QVERIFY(binanceAPI_->isConnected());
    
    // Test authentication (stub)
    bool authenticated = binanceAPI_->authenticate("test_key", "test_secret");
    QVERIFY(authenticated);
    QVERIFY(binanceAPI_->isAuthenticated());
    
    // Test getting instruments
    auto instruments = binanceAPI_->getInstruments();
    QVERIFY(!instruments.empty());
    
    // Test symbol availability
    QVERIFY(binanceAPI_->isSymbolAvailable("BTCUSDT"));
    
    qDebug() << "✓ Exchange API integration test passed";
}

void SystemTest::testPaperTradingMode() {
    qDebug() << "Testing paper trading mode...";
    
    if (tradingEngine_) {
        // Test initial paper mode
        QVERIFY(tradingEngine_->isPaperMode());
        
        // Test switching modes
        tradingEngine_->setPaperMode(false);
        QVERIFY(!tradingEngine_->isPaperMode());
        
        tradingEngine_->setPaperMode(true);
        QVERIFY(tradingEngine_->isPaperMode());
    }
    
    qDebug() << "✓ Paper trading mode test passed";
}

void SystemTest::testEmergencyStop() {
    qDebug() << "Testing emergency stop functionality...";
    
    // Test emergency stop activation
    riskManager_->enableEmergencyStop();
    QVERIFY(riskManager_->isEmergencyStopActive());
    QCOMPARE(riskManager_->getCurrentRiskStatus(), RiskStatus::LIMIT_REACHED);
    
    // Test that orders are rejected during emergency stop
    Order testOrder;
    testOrder.symbol = "EURUSD";
    testOrder.quantity = 0.01;
    
    AccountInfo account;
    account.equity = 10000.0;
    
    std::vector<Position> positions;
    
    bool orderValid = riskManager_->validateOrder(testOrder, account, positions);
    QVERIFY(!orderValid); // Should be rejected due to emergency stop
    
    // Test deactivation
    riskManager_->disableEmergencyStop();
    QVERIFY(!riskManager_->isEmergencyStopActive());
    
    qDebug() << "✓ Emergency stop test passed";
}

void SystemTest::testFullTradingWorkflow() {
    qDebug() << "Testing full trading workflow...";
    
    if (!tradingEngine_) {
        testTradingEngineInitialization();
    }
    
    // Test complete workflow
    QVERIFY(tradingEngine_->initialize());
    
    // Start trading
    bool started = tradingEngine_->start();
    QVERIFY(started);
    QVERIFY(tradingEngine_->isRunning());
    
    // Simulate some trading activity
    auto stats = tradingEngine_->getTradingStats();
    QVERIFY(stats.totalTrades >= 0);
    
    // Stop trading
    tradingEngine_->stop();
    QVERIFY(!tradingEngine_->isRunning());
    
    qDebug() << "✓ Full trading workflow test passed";
}

void SystemTest::testRiskLimitEnforcement() {
    qDebug() << "Testing risk limit enforcement...";
    
    // Create order that would exceed risk limits
    Order highRiskOrder;
    highRiskOrder.symbol = "EURUSD";
    highRiskOrder.quantity = 100.0; // Very large position
    highRiskOrder.price = 1.1000;
    
    AccountInfo smallAccount;
    smallAccount.equity = 1000.0; // Small account
    
    std::vector<Position> positions;
    
    // Should reject high-risk order
    bool orderValid = riskManager_->validateOrder(highRiskOrder, smallAccount, positions);
    QVERIFY(!orderValid);
    
    qDebug() << "✓ Risk limit enforcement test passed";
}

void SystemTest::testPatternToOrderFlow() {
    qDebug() << "Testing pattern to order flow...";
    
    // Create a pattern result
    PatternResult pattern;
    pattern.type = PatternType::SETUP_1_CONSECUTIVE;
    pattern.symbol = "EURUSD";
    pattern.confidence = 0.8;
    pattern.suggestedSide = OrderSide::BUY;
    pattern.suggestedEntry = 1.1010;
    pattern.suggestedStop = 1.0990;
    
    // Generate trading signal
    SymbolConfig config;
    config.symbol = "EURUSD";
    config.riskParams.minLotSize = 0.01;
    
    RenkoChart chart("EURUSD", 0.001);
    TradingSignal signal = patternDetector_->generateSignalFromPattern(pattern, chart, config);
    
    // Verify signal properties
    QCOMPARE(signal.symbol, pattern.symbol);
    QCOMPARE(signal.pattern, pattern.type);
    QCOMPARE(signal.side, pattern.suggestedSide);
    QCOMPARE(signal.entryPrice, pattern.suggestedEntry);
    QCOMPARE(signal.stopLoss, pattern.suggestedStop);
    QVERIFY(signal.takeProfit > signal.entryPrice); // Should set take profit above entry for buy
    
    qDebug() << "✓ Pattern to order flow test passed";
}

void SystemTest::setupTestData() {
    // Setup any test data needed for the tests
    qDebug() << "Setting up test data...";
}

void SystemTest::verifyRenkoBrick(const RenkoBrick& brick, double expectedOpen, double expectedClose, bool expectedIsUp) {
    QCOMPARE(brick.open, expectedOpen);
    QCOMPARE(brick.close, expectedClose);
    QCOMPARE(brick.isUp, expectedIsUp);
}

// Include the MOC file for Qt's meta-object system
QTEST_MAIN(SystemTest)
#include "test_system.moc" 