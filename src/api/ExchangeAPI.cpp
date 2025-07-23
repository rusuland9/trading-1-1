#include "api/ExchangeAPI.h"
#include <iostream>

namespace MasterMind {

// ExchangeAPI base class implementation
ExchangeAPI::ExchangeAPI(Exchange exchangeType)
    : exchangeType_(exchangeType), connected_(false), authenticated_(false) {
    std::cout << "ExchangeAPI base class initialized for exchange type: " 
              << static_cast<int>(exchangeType) << std::endl;
}

void ExchangeAPI::notifyTick(const Tick& tick) {
    if (tickCallback_) {
        tickCallback_(tick);
    }
}

void ExchangeAPI::notifyOrderUpdate(const Order& order) {
    if (orderCallback_) {
        orderCallback_(order);
    }
}

void ExchangeAPI::notifyPositionUpdate(const Position& position) {
    if (positionCallback_) {
        positionCallback_(position);
    }
}

void ExchangeAPI::notifyAccountUpdate(const AccountInfo& account) {
    if (accountCallback_) {
        accountCallback_(account);
    }
}

// WebSocketExchangeAPI implementation
WebSocketExchangeAPI::WebSocketExchangeAPI(Exchange exchangeType)
    : ExchangeAPI(exchangeType), wsConnected_(false) {
    std::cout << "WebSocketExchangeAPI initialized" << std::endl;
}

WebSocketExchangeAPI::~WebSocketExchangeAPI() {
    if (wsConnected_) {
        disconnectWebSocket();
    }
}

// RestExchangeAPI implementation
RestExchangeAPI::RestExchangeAPI(Exchange exchangeType)
    : ExchangeAPI(exchangeType) {
    std::cout << "RestExchangeAPI initialized" << std::endl;
}

RestExchangeAPI::~RestExchangeAPI() = default;

} // namespace MasterMind 