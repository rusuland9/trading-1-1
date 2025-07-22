# Master Mind Trading System

A high-frequency, cross-asset trading system implementing a confidential Renko-based strategy across multiple markets and exchanges.

## ⚠️ CONFIDENTIAL STRATEGY

This trading system implements a proprietary strategy called "Master Mind" with specific entry/exit logic that is confidential. This codebase is for authorized development only.

## 🚀 Features

### Core Trading Strategy
- **Renko-Based Pattern Recognition**: Real-time Renko chart analysis with two main setups
  - **Setup 1**: Two consecutive down bricks + 75% up brick formation
  - **Setup 2**: Green-Red-Green pattern with 75% completion threshold
- **Multi-Asset Support**: Forex, Cryptocurrency, Futures, and Options
- **Dynamic Position Sizing**: Risk-based position sizing with daily limits
- **Advanced Order Management**: Hybrid orders, stop losses, trailing stops

### Risk Management
- **Counter-Based Capital Assessment**: Reassess capital after every N orders (default: 10)
- **Paper Trading Fallback**: Automatic switch after consecutive losses
- **Daily Risk Limits**: 1% or 2% daily risk per user configuration
- **Real-time Drawdown Monitoring**: Automatic protective measures

### Exchange Support
- **Crypto Exchanges**: Binance, Deribit, Coinbase, Delta Exchange
- **Forex Platforms**: MT4/MT5 integration
- **Smart Order Routing**: Optimal execution across venues
- **Low-Latency Architecture**: High-performance C++ implementation

### Advanced Features
- **Real-time Pattern Detection**: Sub-millisecond pattern recognition
- **Audit Trail System**: Complete trade logging and compliance
- **Configuration Management**: JSON/YAML-based settings
- **Web Interface**: Optional web dashboard (configurable)
- **Multi-threading**: Parallel processing for optimal performance

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Trading       │    │   Pattern       │    │   Risk          │
│   Engine        │◄──►│   Detector      │◄──►│   Manager       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                        │                        │
         ▼                        ▼                        ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Order         │    │   Renko         │    │   Config        │
│   Manager       │    │   Charts        │    │   Manager       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                        │                        │
         ▼                        ▼                        ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Exchange      │    │   Database      │    │   Logger &      │
│   APIs          │    │   Manager       │    │   Audit Trail   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 📋 Requirements

### System Requirements
- **OS**: Linux (Ubuntu 18.04+), Windows 10+, macOS 10.14+
- **CPU**: Intel i5 or equivalent (minimum 4 cores recommended)
- **RAM**: 8GB minimum, 16GB recommended
- **Storage**: 10GB free space minimum
- **Network**: Stable internet connection with low latency

### Software Dependencies
- **C++ Compiler**: GCC 8+ or Clang 10+ or MSVC 2019+
- **CMake**: 3.16 or higher
- **OpenSSL**: For secure API communications
- **SQLite**: For local database (included)
- **JSON Library**: For configuration management
- **WebSocket++**: For real-time data feeds
- **Boost Libraries**: For networking and utilities

## 🔧 Installation

### 1. Clone the Repository
```bash
git clone <repository-url>
cd master-mind-trading-system
```

### 2. Install Dependencies

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev libsqlite3-dev libboost-all-dev
```

#### CentOS/RHEL
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake openssl-devel sqlite-devel boost-devel
```

#### Windows (with vcpkg)
```bash
vcpkg install openssl sqlite3 boost-system boost-thread boost-chrono
```

#### macOS (with Homebrew)
```bash
brew install cmake openssl sqlite boost
```

### 3. Build the System
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 4. Install
```bash
sudo make install
```

## ⚙️ Configuration

### 1. Basic Configuration
Edit `config/mastermind_config.json` to set up your trading parameters:

```json
{
  "system": {
    "enablePaperTrading": true,
    "maxThreads": 8
  },
  "globalRiskParameters": {
    "dailyRiskPercent": 0.02,
    "consecutiveLossLimit": 2,
    "ordersPerCounter": 10
  },
  "symbols": [
    {
      "symbol": "EURUSD",
      "capitalAllocation": 2000.0,
      "brickSize": 0.0010,
      "enabled": true
    }
  ]
}
```

### 2. Exchange Configuration
Add your exchange API credentials:

```json
{
  "exchanges": [
    {
      "exchange": "BINANCE",
      "enabled": true,
      "testnet": true,
      "apiKey": "your_api_key",
      "apiSecret": "your_api_secret"
    }
  ]
}
```

### 3. Risk Management Setup
Configure risk parameters per symbol:

```json
{
  "riskParameters": {
    "dailyRiskPercent": 0.01,
    "maxDrawdownPercent": 0.05,
    "paperTradingMode": true
  }
}
```

## 🚀 Usage

### Basic Usage
```bash
# Paper trading mode (safe for testing)
./MasterMindTrader -p

# Live trading mode (real money - use with caution)
./MasterMindTrader -l

# Custom configuration file
./MasterMindTrader -c config/my_config.json

# Verbose logging
./MasterMindTrader -p -v
```

### Command Line Options
```
  -c, --config FILE     Configuration file path
  -p, --paper           Enable paper trading mode
  -l, --live            Enable live trading mode
  -v, --verbose         Enable verbose logging
  -d, --daemon          Run as daemon (background)
  -t, --test            Run system tests
  -h, --help            Show help message
```

### Running Tests
```bash
./MasterMindTrader -t
```

## 📊 Monitoring

### Real-time Status
The system provides real-time status updates including:
- Current risk status
- Total trades and win rate
- Current drawdown
- Profit factor
- Paper/live trading mode indicator

### Log Files
- Main log: `logs/mastermind.log`
- Audit trail: `logs/audit_trail.log`
- Error log: `logs/error.log`

### Trading Reports
Generated reports are saved to:
- Daily reports: `reports/daily_YYYY-MM-DD.json`
- Final report: `reports/final_report.json`

## 🔐 Security Features

### API Security
- Encrypted API key storage
- Request signing for all exchange communications
- Rate limiting and connection pooling
- Automatic reconnection with exponential backoff

### Data Protection
- SQLite database encryption
- Secure audit trail logging
- Configuration file validation
- Memory protection for sensitive data

### Risk Controls
- Emergency stop functionality
- Automatic paper mode fallback
- Position size limits
- Daily risk limits enforcement

## 📈 Strategy Details

### Pattern Recognition
The Master Mind strategy uses two main setups:

1. **Setup 1**: Consecutive brick pattern
   - Detects two consecutive down bricks
   - Waits for 75% completion of up brick
   - Places buy stop order with 2-tick buffer

2. **Setup 2**: Green-Red-Green pattern
   - Identifies 3-brick alternating pattern
   - Confirms 75% completion of final brick
   - Places order 2 ticks beyond pattern

### Exit Logic
Three exit conditions:
1. User-defined profit target hit
2. Opposite signal generated
3. Stop loss triggered

### Risk Management
- Position sizing based on equity and daily risk %
- Counter-based capital reassessment
- Automatic paper mode after consecutive losses
- Real-time drawdown monitoring

## 🛠️ Development

### Building from Source
```bash
git clone <repository>
cd master-mind-trading-system
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Adding New Exchanges
1. Inherit from `ExchangeAPI` base class
2. Implement all virtual methods
3. Add to `ExchangeAPIFactory`
4. Update configuration schema

### Adding New Patterns
1. Extend `PatternDetector` class
2. Implement pattern-specific logic
3. Add to configuration options
4. Update strategy documentation

## ⚠️ Important Warnings

### Live Trading
- **NEVER** run live trading without thorough testing in paper mode
- **VERIFY** all configurations before enabling live mode
- **MONITOR** the system continuously during live trading
- **HAVE** emergency stop procedures in place

### Risk Disclaimers
- Trading involves substantial risk of loss
- Past performance does not guarantee future results
- Only trade with capital you can afford to lose
- This software is provided "AS IS" without warranties

### Legal Notice
- This strategy is confidential and proprietary
- Unauthorized distribution is prohibited
- Usage requires proper licensing agreement
- Compliance with local trading regulations required

## 📞 Support

### Documentation
- Full API documentation in `docs/` directory
- Configuration examples in `config/examples/`
- Strategy specifications in `docs/strategy/`

### Troubleshooting
Common issues and solutions:
1. **Connection errors**: Check API credentials and network
2. **Configuration errors**: Validate JSON syntax
3. **Performance issues**: Adjust thread count and timeouts
4. **Risk alerts**: Review risk parameters and current positions

### Contact
For technical support and licensing inquiries, contact the development team through official channels only.

## 📄 License

This software is proprietary and confidential. All rights reserved.
Unauthorized copying, distribution, or use is strictly prohibited.

---

**Master Mind Trading System v1.0.0**  
*High-Frequency Cross-Asset Trading Platform* 