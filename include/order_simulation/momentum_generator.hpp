#pragma once
#include "i_order_generator.hpp"
#include "market_state.hpp"
#include "benchmark_order.hpp"
#include <random>
#include <optional>

/**
 * @class MomentumGenerator
 * @tparam TOrder
 * @brief Buys continually on upticks, sells continuously on downticks
 */
template<typename TOrder>
class MomentumGenerator : public IOrderGenerator<TOrder, MomentumGenerator<TOrder>> {
public:
    MomentumGenerator(MarketState& market, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();

private:
    MarketState* market_state_;
    double last_price_;
    uint64_t order_id_ = 0;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> offset_dist_{0.01, 0.15}; // small aggressive offset
    std::uniform_int_distribution<uint32_t> qty_dist_{10, 80};
};
