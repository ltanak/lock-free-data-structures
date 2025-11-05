#pragma once
#include <atomic>

struct alignas(64) MarketState {
    std::atomic<double> basePrice{100.0};
    std::atomic<double> volatility{0.1};
    std::atomic<double> trendBias{0.0};
    std::atomic<uint64_t> lastOrderId{0};

    double getPrice() const noexcept;
    double getVolatility() const noexcept;
    double getBias() const noexcept;

    
};