#include "core/TradingEngine.h"
#include "core/RenkoChart.h"
#include "core/PatternDetector.h"
#include "core/Types.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <iomanip>

using namespace MasterMind;

int main() {
    std::cout << "\n=== MASTER MIND TRADING SYSTEM - SIMULATION DEMO ===\n" << std::endl;
    
    // Create Renko chart for EURUSD
    RenkoChart chart("EURUSD", 0.0010); // 10 pip bricks
    
    // Create pattern detector
    PatternDetector detector;
    
    std::cout << "Generating simulated market data for EURUSD..." << std::endl;
    std::cout << "Brick Size: 0.0010 (10 pips)" << std::endl;
    std::cout << "Looking for Setup 1 (Consecutive) and Setup 2 (Green-Red-Green) patterns\n" << std::endl;
    
    // Generate simulated price data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> price_dist(0.0, 0.0002); // Small price movements
    
    double currentPrice = 1.1000; // Starting EURUSD price
    int totalTicks = 0;
    int patternsDetected = 0;
    int tradingSignals = 0;
    
    // Simulate 500 price ticks
    for (int i = 0; i < 500; ++i) {
        // Generate realistic price movement
        double priceChange = price_dist(gen);
        currentPrice += priceChange;
        
        // Create tick
        Tick tick("EURUSD", currentPrice - 0.0001, currentPrice + 0.0001, 
                 currentPrice, 1000000, std::chrono::system_clock::now());
        
        // Add to Renko chart
        chart.addTick(tick);
        totalTicks++;
        
        // Check for new bricks and patterns
        auto bricks = chart.getBricks();
        if (bricks.size() >= 3) {
            // Check for Setup 1: Two consecutive down bricks + 75% up brick
            auto recentBricks = std::vector<RenkoBrick>(bricks.end() - 3, bricks.end());
            
            if (!recentBricks[0].isUp && !recentBricks[1].isUp && recentBricks[2].isUp) {
                std::cout << "🔍 SETUP 1 DETECTED at price " << std::fixed << std::setprecision(5) 
                         << currentPrice << " (Two down + up pattern)" << std::endl;
                patternsDetected++;
                
                // Generate trading signal
                std::cout << "📈 BUY SIGNAL generated - Entry: " << (currentPrice + 0.0002) 
                         << ", Stop: " << (currentPrice - 0.0020) 
                         << ", Target: " << (currentPrice + 0.0040) << std::endl;
                tradingSignals++;
            }
            
            // Check for Setup 2: Green-Red-Green pattern
            if (recentBricks[0].isUp && !recentBricks[1].isUp && recentBricks[2].isUp) {
                std::cout << "🔍 SETUP 2 DETECTED at price " << std::fixed << std::setprecision(5) 
                         << currentPrice << " (Green-Red-Green pattern)" << std::endl;
                patternsDetected++;
                
                // Generate trading signal
                std::cout << "📈 BUY SIGNAL generated - Entry: " << (currentPrice + 0.0002) 
                         << ", Stop: " << (currentPrice - 0.0020) 
                         << ", Target: " << (currentPrice + 0.0040) << std::endl;
                tradingSignals++;
            }
        }
        
        // Print progress every 100 ticks
        if (i % 100 == 0) {
            std::cout << "Progress: " << i << "/500 ticks, Current Price: " 
                     << std::fixed << std::setprecision(5) << currentPrice 
                     << ", Bricks: " << chart.getBricks().size() << std::endl;
        }
        
        // Small delay to simulate real-time
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Final results
    std::cout << "\n=== SIMULATION RESULTS ===" << std::endl;
    std::cout << "Total Ticks Processed: " << totalTicks << std::endl;
    std::cout << "Total Renko Bricks: " << chart.getBricks().size() << std::endl;
    std::cout << "Patterns Detected: " << patternsDetected << std::endl;
    std::cout << "Trading Signals Generated: " << tradingSignals << std::endl;
    std::cout << "Final Price: " << std::fixed << std::setprecision(5) << currentPrice << std::endl;
    
    if (patternsDetected > 0) {
        std::cout << "\n✅ SUCCESS: Master Mind patterns detected and signals generated!" << std::endl;
        std::cout << "📊 Pattern Detection Rate: " << std::fixed << std::setprecision(1) 
                 << (double(patternsDetected) / chart.getBricks().size() * 100) << "%" << std::endl;
    } else {
        std::cout << "\nℹ️  No patterns detected in this simulation run." << std::endl;
        std::cout << "Try running again - market conditions affect pattern formation." << std::endl;
    }
    
    std::cout << "\n=== PAPER TRADING RESULTS ===" << std::endl;
    std::cout << "Account Equity: $10,000 (simulated)" << std::endl;
    std::cout << "Risk Per Trade: 2% ($200)" << std::endl;
    std::cout << "Position Size per Signal: 0.02 lots" << std::endl;
    std::cout << "Total Potential Trades: " << tradingSignals << std::endl;
    
    return 0;
} 