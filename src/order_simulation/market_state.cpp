#include "market_state.hpp"



double MarketState::getPrice() const noexcept {
    return basePrice.load(std::memory_order_relaxed);
}

double MarketState::getVolatility() const noexcept {
    return volatility.load(std::memory_order_relaxed);
}

double MarketState::getBias() const noexcept {
    return trendBias.load(std::memory_order_relaxed);
}