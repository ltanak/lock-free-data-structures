#include "order_simulation/market_maker_generator.hpp"
#include "utils/timing.hpp"

template<typename TOrder>
MarketMakerGenerator<TOrder>::MarketMakerGenerator(MarketState& market, std::optional<uint32_t> seed) : market_state_(&market) {
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
}

template<typename TOrder>
TOrder MarketMakerGenerator<TOrder>::generateOrder() {
    order_id_++;
    
    double mid = market_state_->getPrice();
    double spread = market_state_->getSpread();
    double spread_offset = spread * spread_frac_(rng_);
    
    // quote near mid price with tight spread (provides liquidity)
    bool is_bid = side_dist_(rng_) == 0;
    // if bid, quote below mid; if ask, quote above mid
    double price = is_bid ? mid - spread_offset : mid + spread_offset;
    price = std::round(price * 100.0) / 100.0;
    
    OrderType type = is_bid ? OrderType::BUY : OrderType::SELL;
    uint32_t quantity = qty_dist_(rng_);
    uint64_t timestamp = ltime::rdtsc_lfence();
    
    return TOrder{order_id_, type, price, quantity, timestamp};
}

template class MarketMakerGenerator<BenchmarkOrder>;
