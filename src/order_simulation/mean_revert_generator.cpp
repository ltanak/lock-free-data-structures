#include "order_simulation/mean_revert_generator.hpp"
#include "utils/timing.hpp"

template<typename TOrder>
MeanRevertGenerator<TOrder>::MeanRevertGenerator(MarketState& market, double equilibrium, std::optional<uint32_t> seed)
    : market_state_(&market), equilibrium_price_(equilibrium) {
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
}

template<typename TOrder>
TOrder MeanRevertGenerator<TOrder>::generateOrder() {
    order_id_++;
    
    double current_price = market_state_->getPrice();
    double deviation = current_price - equilibrium_price_;
    
    // sell if price too high, buy if too low
    bool fade_high = deviation > 0.0;
    OrderType type = fade_high ? OrderType::SELL : OrderType::BUY;
    
    double fade_amount = fade_dist_(rng_) * std::abs(deviation);
    double price = fade_high ? current_price - fade_amount : current_price + fade_amount;
    price = std::ceil(price * 100.0) / 100.0;
    
    uint32_t quantity = qty_dist_(rng_);
    uint64_t timestamp = ltime::rdtsc_lfence();
    
    return TOrder{order_id_, type, price, quantity, timestamp};
}

template class MeanRevertGenerator<BenchmarkOrder>;
