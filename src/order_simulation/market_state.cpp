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

void MarketState::applyEvent(double priceMult, double newVol, double bias) noexcept {
    basePrice.store(basePrice.load(std::memory_order_relaxed) * priceMult, std::memory_order_relaxed);
    volatility.store(newVol, std::memory_order_relaxed);
    trendBias.store(bias, std::memory_order_relaxed);
}