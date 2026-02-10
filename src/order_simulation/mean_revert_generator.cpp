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
    double spread = market_state_->getSpread();
    double deviation = current_price - equilibrium_price_;
    
    // sell if price too high, buy if too low (fade the move)
    // dead zone no fade on small deviations
    bool fade_high = deviation > spread;
    bool fade_low = deviation < -spread;
    
    OrderType type;
    double price;
    
    if (fade_high) {
        type = OrderType::SELL;
        double fade_amount = fade_dist_(rng_) * std::abs(deviation);
        price = current_price - fade_amount;  // sell below current but above equilibrium
    } else if (fade_low) {
        type = OrderType::BUY;
        double fade_amount = fade_dist_(rng_) * std::abs(deviation);
        price = current_price + fade_amount;  // buy above current but below equilibrium
    } else {
        // near equilibrium market make
        std::uniform_int_distribution<int> side{0, 1};
        bool is_buy = side(rng_) == 0;
        type = is_buy ? OrderType::BUY : OrderType::SELL;
        price = is_buy ? current_price - spread : current_price + spread;
    }
    
    price = std::round(price * 100.0) / 100.0;
    
    uint32_t quantity = qty_dist_(rng_);
    uint64_t timestamp = ltime::rdtsc_lfence();
    
    return TOrder{order_id_, type, price, quantity, timestamp};
}

template class MeanRevertGenerator<BenchmarkOrder>;
