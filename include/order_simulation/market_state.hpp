#pragma once
#include <atomic>

/**
 * @brief each generator reads from the market state to inform its order generation
 * atomic variables for thread-safe access and modification
 * aligned to cache line size (prevents false sharing)
 */

struct alignas(64) MarketState {
    std::atomic<double> basePrice{100.0};
    std::atomic<double> volatility{0.1};
    std::atomic<double> trendBias{0.0};
    std::atomic<uint64_t> lastOrderId{0};

    double getPrice() const noexcept;
    double getVolatility() const noexcept;
    double getBias() const noexcept;
    void applyEvent(double priceMult, double newVol, double bias) noexcept;
};

/**
 * TO IMPLEMENT IN THE FUTURE
 * MarketEvent struct to represent market events that can impact the market state (e.g., surge, crash, etc)
 * more complex functions to apply events to the market state
 */