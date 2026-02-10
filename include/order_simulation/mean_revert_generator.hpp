#pragma once
#include "i_order_generator.hpp"
#include "market_state.hpp"
#include "benchmark_order.hpp"
#include <random>
#include <optional>

template<typename TOrder>
class MeanRevertGenerator : public IOrderGenerator<TOrder, MeanRevertGenerator<TOrder>> {
public:
    MeanRevertGenerator(MarketState& market, double equilibrium, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();

private:
    MarketState* market_state_;
    double equilibrium_price_;
    uint64_t order_id_ = 0;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> fade_dist_{0.05, 0.5}; // fraction of deviation to fade
    std::uniform_int_distribution<uint32_t> qty_dist_{5, 40};
};
