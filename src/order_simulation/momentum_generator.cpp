#include "order_simulation/momentum_generator.hpp"
#include "utils/timing.hpp"

template<typename TOrder>
MomentumGenerator<TOrder>::MomentumGenerator(MarketState& market, std::optional<uint32_t> seed)
    : market_state_(&market), last_price_(market.getPrice()) {
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
}

template<typename TOrder>
TOrder MomentumGenerator<TOrder>::generateOrder() {
    order_id_++;
    
    double current_price = market_state_->getPrice();
    double spread = market_state_->getSpread();
    double bias = market_state_->getBias();
    
    // buy on upticks, sell on downticks (trend following)
    bool buy_on_momentum = (current_price > last_price_) && (bias >= 0.5);
    bool sell_on_momentum = (current_price < last_price_) && (bias < 0.5);
    
    bool is_buy = buy_on_momentum || (!sell_on_momentum && bias > 0.5);
    OrderType type = is_buy ? OrderType::BUY : OrderType::SELL;
    
    // Momentum traders are aggressive: they cross the spread
    double offset = offset_dist_(rng_) + spread;
    double price;
    if (is_buy) {
        price = current_price + offset;   // aggressive buy above mid
    } else {
        price = current_price - offset;   // aggressive sell below mid
    }
    price = std::round(price * 100.0) / 100.0;
    
    uint32_t quantity = qty_dist_(rng_);
    uint64_t timestamp = ltime::rdtsc_lfence();
    
    last_price_ = current_price;
    return TOrder{order_id_, type, price, quantity, timestamp};
}

template class MomentumGenerator<BenchmarkOrder>;
