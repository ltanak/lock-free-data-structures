#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/market_state.hpp"
#include "utils/timing.hpp"
#include <random>
#include <chrono>
#include <optional>

template<typename TOrder>
RandomOrderGenerator<TOrder>::RandomOrderGenerator(MarketState &market, uint32_t maxQuantity, std::optional<uint32_t> seed)
 : maxQuantity_(maxQuantity), marketState_(&market)
{
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
    quantityDist_ = std::uniform_int_distribution<uint32_t>(1, maxQuantity_);
    sideDist_ = std::uniform_int_distribution<int>(0, 1);
}

template<typename TOrder>
TOrder RandomOrderGenerator<TOrder>::generateOrder(){
    orderId_++;

    double base = marketState_->getPrice();
    double vol = marketState_->getVolatility();
    double bias = marketState_->getBias();
    double spread = marketState_->getSpread();

    // use spread as the main scale, with volatility adding extra noise
    // orders cluster near the mid price within a few spread-widths
    std::normal_distribution<double> spreadDist(0.0, spread * (1.0 + vol * 50.0));
    double offset = spreadDist(rng_);
    
    // probability based on bias (bias > 0.5 favors buys)
    std::uniform_real_distribution<double> bias_dist(0.0, 1.0);
    bool isBuy = bias_dist(rng_) < bias;
    OrderType type = isBuy ? OrderType::BUY : OrderType::SELL;
    
    // buys typically price at or below mid, sells at or above
    double price = base;
    price += (isBuy) ? - std::abs(offset) : std::abs(offset);
    price = std::round(price * 100.0) / 100.0;

    uint32_t quantity = quantityDist_(rng_);
    uint64_t timestamp = ltime::rdtsc_lfence();

    return TOrder{orderId_, type, price, quantity, timestamp};
}

template class RandomOrderGenerator<BenchmarkOrder>;
