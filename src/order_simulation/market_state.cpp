#include "order_simulation/market_state.hpp"

double MarketState::getPrice() const noexcept {
    return base_price.load(std::memory_order_relaxed);
}

double MarketState::getVolatility() const noexcept {
    return volatility.load(std::memory_order_relaxed);
}

double MarketState::getTrend() const noexcept {
    return trend.load(std::memory_order_relaxed);
}

double MarketState::getDrift() const noexcept {
    return drift.load(std::memory_order_relaxed);
}

double MarketState::getSpread() const noexcept {
    return spread.load(std::memory_order_relaxed);
}

void MarketState::applyEvent(double priceMult, double newVol, double bias) noexcept {
    base_price.store(base_price.load(std::memory_order_relaxed) * priceMult, std::memory_order_relaxed);
    volatility.store(newVol, std::memory_order_relaxed);
    trend.store(bias, std::memory_order_relaxed);
}

