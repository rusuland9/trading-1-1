#include "core/RenkoChart.h"
#include <iostream>
#include <algorithm>

namespace MasterMind {

RenkoChart::RenkoChart(const Symbol& symbol, double brickSize, size_t maxBricks)
    : symbol_(symbol), brickSize_(brickSize), tickValue_(0.0001), maxBricks_(maxBricks),
      currentBrickInitialized_(false), lastPrice_(0), highestPrice_(0), lowestPrice_(0) {
    
    std::cout << "RenkoChart created for " << symbol << " with brick size " << brickSize << std::endl;
}

RenkoChart::~RenkoChart() = default;

void RenkoChart::addTick(const Tick& tick) {
    addPrice(tick.last, tick.timestamp);
}

void RenkoChart::addPrice(Price price, TimePoint timestamp) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (price <= 0) {
        return;
    }
    
    lastPrice_ = price;
    lastUpdate_ = timestamp;
    
    // Initialize current brick if needed
    if (!currentBrickInitialized_) {
        initializeCurrentBrick(price, timestamp);
        return;
    }
    
    // Update high/low tracking
    highestPrice_ = std::max(highestPrice_, price);
    lowestPrice_ = std::min(lowestPrice_, price);
    
    // Try to form new bricks
    updateCurrentBrick(price, timestamp);
}

void RenkoChart::setBrickSize(double brickSize) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    if (brickSize > 0) {
        brickSize_ = brickSize;
        std::cout << "Brick size updated to " << brickSize << " for " << symbol_ << std::endl;
    }
}

double RenkoChart::getBrickSize() const {
    return brickSize_;
}

std::vector<RenkoBrick> RenkoChart::getBricks(size_t count) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (count == 0 || count >= bricks_.size()) {
        return std::vector<RenkoBrick>(bricks_.begin(), bricks_.end());
    }
    
    auto startIt = bricks_.end() - count;
    return std::vector<RenkoBrick>(startIt, bricks_.end());
}

std::vector<RenkoBrick> RenkoChart::getLastNBricks(size_t n) const {
    return getBricks(n);
}

RenkoBrick RenkoChart::getLastBrick() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (bricks_.empty()) {
        return RenkoBrick();
    }
    
    return bricks_.back();
}

RenkoBrick RenkoChart::getCurrentBrick() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return currentBrick_;
}

size_t RenkoChart::getBrickCount() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return bricks_.size();
}

bool RenkoChart::hasConsecutiveDownBricks(int count) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (static_cast<int>(bricks_.size()) < count) {
        return false;
    }
    
    for (int i = 1; i <= count; ++i) {
        if (bricks_[bricks_.size() - i].isUp) {
            return false;
        }
    }
    
    return true;
}

bool RenkoChart::hasConsecutiveUpBricks(int count) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (static_cast<int>(bricks_.size()) < count) {
        return false;
    }
    
    for (int i = 1; i <= count; ++i) {
        if (!bricks_[bricks_.size() - i].isUp) {
            return false;
        }
    }
    
    return true;
}

bool RenkoChart::hasGreenRedGreenPattern() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (bricks_.size() < 3) {
        return false;
    }
    
    size_t idx = bricks_.size() - 3;
    return bricks_[idx].isUp && !bricks_[idx + 1].isUp && bricks_[idx + 2].isUp;
}

bool RenkoChart::hasRedGreenRedPattern() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (bricks_.size() < 3) {
        return false;
    }
    
    size_t idx = bricks_.size() - 3;
    return !bricks_[idx].isUp && bricks_[idx + 1].isUp && !bricks_[idx + 2].isUp;
}

double RenkoChart::getPartialBrickCompletion() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return currentBrick_.completionPercent;
}

Price RenkoChart::getNextUpBrickLevel() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (bricks_.empty()) {
        return lastPrice_ + brickSize_;
    }
    
    return bricks_.back().close + brickSize_;
}

Price RenkoChart::getNextDownBrickLevel() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (bricks_.empty()) {
        return lastPrice_ - brickSize_;
    }
    
    return bricks_.back().close - brickSize_;
}

Price RenkoChart::calculateSetup1EntryPrice(OrderSide side, int tickBuffer) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    Price basePrice = (side == OrderSide::BUY) ? getNextUpBrickLevel() : getNextDownBrickLevel();
    double tickAdjustment = tickBuffer * tickValue_;
    
    return (side == OrderSide::BUY) ? basePrice + tickAdjustment : basePrice - tickAdjustment;
}

Price RenkoChart::calculateSetup2EntryPrice(OrderSide side, int tickBuffer) const {
    // Similar to Setup 1 for now (can be refined)
    return calculateSetup1EntryPrice(side, tickBuffer);
}

Price RenkoChart::calculateStopLoss(OrderSide side, int tickBuffer) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    if (bricks_.empty()) {
        return lastPrice_;
    }
    
    Price stopLevel;
    if (side == OrderSide::BUY) {
        stopLevel = bricks_.back().close - brickSize_;
    } else {
        stopLevel = bricks_.back().close + brickSize_;
    }
    
    double tickAdjustment = tickBuffer * tickValue_;
    return (side == OrderSide::BUY) ? stopLevel - tickAdjustment : stopLevel + tickAdjustment;
}

bool RenkoChart::isUpBrickForming() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return currentBrick_.isUp && currentBrick_.completionPercent > 0;
}

bool RenkoChart::isDownBrickForming() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return !currentBrick_.isUp && currentBrick_.completionPercent > 0;
}

bool RenkoChart::isBrickComplete(double completionThreshold) const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return currentBrick_.completionPercent >= completionThreshold;
}

Symbol RenkoChart::getSymbol() const {
    return symbol_;
}

TimePoint RenkoChart::getLastUpdateTime() const {
    return lastUpdate_;
}

void RenkoChart::setTickValue(double tickValue) {
    tickValue_ = tickValue;
}

double RenkoChart::getTickValue() const {
    return tickValue_;
}

void RenkoChart::reset() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    bricks_.clear();
    currentBrick_ = RenkoBrick();
    currentBrickInitialized_ = false;
    std::cout << "RenkoChart reset for " << symbol_ << std::endl;
}

// Private methods
void RenkoChart::initializeCurrentBrick(Price price, TimePoint timestamp) {
    currentBrick_.open = price;
    currentBrick_.close = price;
    currentBrick_.high = price;
    currentBrick_.low = price;
    currentBrick_.timestamp = timestamp;
    currentBrick_.isUp = true;  // Default assumption
    currentBrick_.completionPercent = 0.0;
    
    highestPrice_ = price;
    lowestPrice_ = price;
    currentBrickInitialized_ = true;
}

void RenkoChart::updateCurrentBrick(Price price, TimePoint timestamp) {
    // Simple brick formation logic
    if (bricks_.empty()) {
        // First brick logic
        if (price >= currentBrick_.open + brickSize_) {
            // Form up brick
            finalizeBrick(RenkoBrick(currentBrick_.open, currentBrick_.open + brickSize_, timestamp, true));
        } else if (price <= currentBrick_.open - brickSize_) {
            // Form down brick
            finalizeBrick(RenkoBrick(currentBrick_.open, currentBrick_.open - brickSize_, timestamp, false));
        } else {
            // Update partial completion
            double upDistance = price - currentBrick_.open;
            double downDistance = currentBrick_.open - price;
            
            if (upDistance > downDistance) {
                currentBrick_.isUp = true;
                currentBrick_.completionPercent = upDistance / brickSize_;
            } else {
                currentBrick_.isUp = false;
                currentBrick_.completionPercent = downDistance / brickSize_;
            }
        }
    } else {
        // Subsequent bricks
        Price lastClose = bricks_.back().close;
        
        if (price >= lastClose + brickSize_) {
            // Form up brick
            finalizeBrick(RenkoBrick(lastClose, lastClose + brickSize_, timestamp, true));
        } else if (price <= lastClose - brickSize_) {
            // Form down brick
            finalizeBrick(RenkoBrick(lastClose, lastClose - brickSize_, timestamp, false));
        } else {
            // Update partial completion
            double upDistance = price - lastClose;
            double downDistance = lastClose - price;
            
            if (std::abs(upDistance) > std::abs(downDistance)) {
                currentBrick_.isUp = true;
                currentBrick_.completionPercent = upDistance / brickSize_;
            } else {
                currentBrick_.isUp = false;
                currentBrick_.completionPercent = std::abs(downDistance) / brickSize_;
            }
            
            currentBrick_.completionPercent = std::max(0.0, std::min(1.0, currentBrick_.completionPercent));
        }
    }
}

void RenkoChart::finalizeBrick(const RenkoBrick& brick) {
    bricks_.push_back(brick);
    
    // Start new current brick
    currentBrick_ = RenkoBrick();
    currentBrick_.open = brick.close;
    currentBrick_.close = brick.close;
    currentBrick_.timestamp = brick.timestamp;
    currentBrick_.completionPercent = 0.0;
    
    // Reset price tracking
    highestPrice_ = brick.close;
    lowestPrice_ = brick.close;
    
    // Maintain maximum bricks
    maintainMaxBricks();
    
    std::cout << "New " << (brick.isUp ? "UP" : "DOWN") << " brick formed for " 
              << symbol_ << " at " << brick.close << std::endl;
}

void RenkoChart::maintainMaxBricks() {
    while (bricks_.size() > maxBricks_) {
        bricks_.pop_front();
    }
}

// Stub implementations for other methods
double RenkoChart::getAverageBrickTime() const { return 300.0; } // 5 minutes average
int RenkoChart::getConsecutiveUpCount() const { 
    int count = 0;
    for (auto it = bricks_.rbegin(); it != bricks_.rend() && it->isUp; ++it) {
        count++;
    }
    return count;
}

int RenkoChart::getConsecutiveDownCount() const {
    int count = 0;
    for (auto it = bricks_.rbegin(); it != bricks_.rend() && !it->isUp; ++it) {
        count++;
    }
    return count;
}

void RenkoChart::clearOldBricks(size_t keepCount) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    while (bricks_.size() > keepCount) {
        bricks_.pop_front();
    }
}

void RenkoChart::setMaxBricks(size_t maxBricks) {
    maxBricks_ = maxBricks;
}

Price RenkoChart::getBrickHighPrice(const RenkoBrick& brick) const {
    return brick.high;
}

Price RenkoChart::getBrickLowPrice(const RenkoBrick& brick) const {
    return brick.low;
}

} // namespace MasterMind 