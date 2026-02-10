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
    double bias = market_state_->getBias();
    
    // buy on upticks, sell on downticks (store prev price and bias)
    bool buy_on_momentum = (current_price > last_price_) && (bias > 0.5);
    bool sell_on_momentum = (current_price < last_price_) && (bias < 0.5);
    
    bool is_buy = buy_on_momentum || (!sell_on_momentum && bias > 0.5);
    OrderType type = is_buy ? OrderType::BUY : OrderType::SELL;
    
    double price = current_price + offset_dist_(rng_);
    price = std::ceil(price * 100.0) / 100.0;
    
    uint32_t quantity = qty_dist_(rng_);
    uint64_t timestamp = ltime::rdtsc_lfence();
    
    last_price_ = current_price;
    return TOrder{order_id_, type, price, quantity, timestamp};
}

template class MomentumGenerator<BenchmarkOrder>;
