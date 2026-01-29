#pragma once
#include <atomic>
#include <random>
#include <chrono>
#include <optional>

/**
 *  - 15% volatility
 * MEDIUM = 15 - 25%
 * HIGH = 25 - 35%  
 */

enum class Volatility {
    LOW, MEDIUM, HIGH
};

enum class MarketEvent {
    BASE,
    UP_TREND,
    DOWN_TREND,
    CRASH,
    PULL_BACK,
    SURGE
};

/**
 * @brief each generator reads from the market state to inform its order generation
 * atomic variables for thread-safe access and modification
 * aligned to cache line size (prevents false sharing)
 */

struct alignas(64) MarketState {
    std::atomic<double> base_price{100.0};
    std::atomic<double> volatility{0.2};
    std::atomic<double> trend{0.0};
    std::atomic<double> drift{0.0};
    std::atomic<double> spread{0.0};

    // std::atomic<uint64_t> lastOrderId{0};
    std::atomic<Volatility> curr_volatility{Volatility::MEDIUM};
    std::uniform_int_distribution<int> low_vol_dist; // 10 - 15%
    std::uniform_int_distribution<int> med_vol_dist; // 15 - 25%
    std::uniform_int_distribution<int> high_vol_dist; // 25 - 35%

    // marketevents
    std::atomic<MarketEvent> event{MarketEvent::BASE};

    double getPrice() const noexcept;
    double getVolatility() const noexcept;
    double getTrend() const noexcept;
    double getDrift() const noexcept;
    double getSpread() const noexcept;
    void applyEvent(double priceMult, double newVol, double bias) noexcept;

};
