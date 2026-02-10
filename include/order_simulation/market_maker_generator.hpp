#pragma once

#include "i_order_generator.hpp"
#include "market_state.hpp"
#include "benchmark_order.hpp"
#include <random>
#include <optional>

template<typename TOrder>
class MarketMakerGenerator : public IOrderGenerator<TOrder, MarketMakerGenerator<TOrder>> {

public:
    MarketMakerGenerator(MarketState &market, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();
private:

    uint64_t order_id_ = 0;
    MarketState *market_state_;

    std::mt19937 rng_;
    std::uniform_real_distribution<double> spread_dist_{0.5, 2.0};
    std::uniform_int_distribution<uint32_t> qty_dist_{1, 20};
    std::uniform_int_distribution<int> side_dist_{0, 1};
};
