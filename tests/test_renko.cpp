#include <QTest>
#include <QSignalSpy>
#include <QDateTime>
#include "RenkoChart.h"

class TestRenko : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Basic functionality tests
    void testBrickCreation();
    void testBrickSize();
    void testPriceUpdates();
    void testPatternDetection();
    void testRiskManagement();
    void testSetup1Pattern();
    void testSetup2Pattern();
    
    // Edge cases
    void testZeroPriceUpdate();
    void testNegativePriceUpdate();
    void testLargePriceJumps();
    void testRapidUpdates();

private:
    MasterMind::RenkoChart* renkoChart_;
};

void TestRenko::initTestCase() {
    // This will be called once before the first test function is executed
}

void TestRenko::cleanupTestCase() {
    // This will be called once after the last test function was executed
}

void TestRenko::init() {
    // This will be called before each test function is executed
    renkoChart_ = new MasterMind::RenkoChart(0.001); // 1 pip brick size for EURUSD
}

void TestRenko::cleanup() {
    // This will be called after every test function
    delete renkoChart_;
    renkoChart_ = nullptr;
}

void TestRenko::testBrickCreation() {
    QVERIFY(renkoChart_ != nullptr);
    QCOMPARE(renkoChart_->getBrickSize(), 0.001);
    
    // Test initial state
    QCOMPARE(renkoChart_->getBrickCount(), 0);
    
    // Add first price point to establish reference
    renkoChart_->addPrice(1.1000, QDateTime::currentDateTime());
    
    // Should still be 0 bricks as we need price movement
    QCOMPARE(renkoChart_->getBrickCount(), 0);
    
    // Add price that should create an up brick
    renkoChart_->addPrice(1.1015, QDateTime::currentDateTime());
    
    // Should now have at least one brick
    QVERIFY(renkoChart_->getBrickCount() > 0);
}

void TestRenko::testBrickSize() {
    double originalSize = 0.001;
    QCOMPARE(renkoChart_->getBrickSize(), originalSize);
    
    // Test changing brick size
    double newSize = 0.002;
    renkoChart_->setBrickSize(newSize);
    QCOMPARE(renkoChart_->getBrickSize(), newSize);
    
    // Test invalid brick size (should not change)
    renkoChart_->setBrickSize(-0.001);
    QCOMPARE(renkoChart_->getBrickSize(), newSize); // Should remain unchanged
    
    renkoChart_->setBrickSize(0.0);
    QCOMPARE(renkoChart_->getBrickSize(), newSize); // Should remain unchanged
}

void TestRenko::testPriceUpdates() {
    QSignalSpy brickSpy(renkoChart_, &MasterMind::RenkoChart::brickCreated);
    
    double basePrice = 1.1000;
    renkoChart_->addPrice(basePrice, QDateTime::currentDateTime());
    
    // No brick should be created yet
    QCOMPARE(brickSpy.count(), 0);
    
    // Add price that creates up bricks
    renkoChart_->addPrice(basePrice + 0.0025, QDateTime::currentDateTime());
    
    // Should have created 2 up bricks (2.5 pips = 2 bricks of 1 pip each)
    QCOMPARE(brickSpy.count(), 2);
    
    // Verify brick properties
    auto bricks = renkoChart_->getBricks();
    QVERIFY(!bricks.isEmpty());
    QVERIFY(bricks.last().isUp);
}

void TestRenko::testPatternDetection() {
    QSignalSpy patternSpy(renkoChart_, &MasterMind::RenkoChart::patternDetected);
    
    double basePrice = 1.1000;
    renkoChart_->addPrice(basePrice, QDateTime::currentDateTime());
    
    // Create Setup 1 pattern: Two down bricks + 75% up brick
    renkoChart_->addPrice(basePrice - 0.002, QDateTime::currentDateTime()); // 2 down bricks
    renkoChart_->addPrice(basePrice - 0.002 + 0.00075, QDateTime::currentDateTime()); // 75% up
    
    // Should detect Setup 1 pattern
    QVERIFY(patternSpy.count() > 0);
    
    // Check if Setup 1 was detected
    bool setup1Detected = false;
    for (const auto& args : patternSpy) {
        if (args.at(0).toString().contains("Setup 1")) {
            setup1Detected = true;
            break;
        }
    }
    QVERIFY(setup1Detected);
}

void TestRenko::testRiskManagement() {
    // Test that risk management parameters are respected
    renkoChart_->setMaxRiskPercent(0.02); // 2% max risk
    renkoChart_->setCapital(10000.0); // $10,000 capital
    
    QCOMPARE(renkoChart_->getMaxRiskPercent(), 0.02);
    QCOMPARE(renkoChart_->getCapital(), 10000.0);
    
    // Test position sizing calculation
    double positionSize = renkoChart_->calculatePositionSize(0.001, 0.01); // 1 pip stop, 1% risk
    QVERIFY(positionSize > 0);
    QVERIFY(positionSize <= renkoChart_->getCapital() * 0.01 / 0.001); // Should not exceed risk limit
}

void TestRenko::testSetup1Pattern() {
    // Specific test for Setup 1: Two consecutive down bricks + 75% up brick
    double basePrice = 1.1000;
    renkoChart_->addPrice(basePrice, QDateTime::currentDateTime());
    
    // Create exactly two down bricks
    renkoChart_->addPrice(basePrice - 0.002, QDateTime::currentDateTime());
    
    // Now add 75% of an up brick
    double targetPrice = basePrice - 0.002 + (0.001 * 0.75);
    renkoChart_->addPrice(targetPrice, QDateTime::currentDateTime());
    
    // Verify Setup 1 detection
    QVERIFY(renkoChart_->isSetup1Pattern());
}

void TestRenko::testSetup2Pattern() {
    // Specific test for Setup 2: Green-Red-Green with 75% completion
    double basePrice = 1.1000;
    renkoChart_->addPrice(basePrice, QDateTime::currentDateTime());
    
    // Create Green brick
    renkoChart_->addPrice(basePrice + 0.001, QDateTime::currentDateTime());
    
    // Create Red brick
    renkoChart_->addPrice(basePrice, QDateTime::currentDateTime());
    
    // Create 75% of Green brick
    renkoChart_->addPrice(basePrice + (0.001 * 0.75), QDateTime::currentDateTime());
    
    // Verify Setup 2 detection
    QVERIFY(renkoChart_->isSetup2Pattern());
}

void TestRenko::testZeroPriceUpdate() {
    // Test that zero price updates are handled gracefully
    renkoChart_->addPrice(0.0, QDateTime::currentDateTime());
    QCOMPARE(renkoChart_->getBrickCount(), 0);
}

void TestRenko::testNegativePriceUpdate() {
    // Test that negative prices are handled (though unrealistic for forex)
    renkoChart_->addPrice(-1.0, QDateTime::currentDateTime());
    QCOMPARE(renkoChart_->getBrickCount(), 0);
}

void TestRenko::testLargePriceJumps() {
    // Test handling of large price jumps that would create many bricks
    double basePrice = 1.1000;
    renkoChart_->addPrice(basePrice, QDateTime::currentDateTime());
    
    // Large jump that should create 100 bricks
    renkoChart_->addPrice(basePrice + 0.1, QDateTime::currentDateTime());
    
    QCOMPARE(renkoChart_->getBrickCount(), 100);
}

void TestRenko::testRapidUpdates() {
    // Test rapid price updates to ensure performance
    double basePrice = 1.1000;
    
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < 10000; ++i) {
        double price = basePrice + (i * 0.00001); // Small incremental moves
        renkoChart_->addPrice(price, QDateTime::currentDateTime());
    }
    
    auto endTime = QDateTime::currentMSecsSinceEpoch();
    auto duration = endTime - startTime;
    
    // Should complete within reasonable time (less than 1 second)
    QVERIFY(duration < 1000);
}

QTEST_MAIN(TestRenko)
#include "test_renko.moc" 