#pragma once
#include "i_order_generator.hpp"
#include "market_state.hpp"
#include <random>
#include <optional>

template <typename TOrder>
class RandomOrderGenerator : public IOrderGenerator<TOrder, RandomOrderGenerator<TOrder>> {
    
public:
    RandomOrderGenerator(MarketState &market, double maxQuantity, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();
private:
    double maxQuantity_;
    uint64_t orderId_ = 0;
    MarketState *marketState_;

    std::mt19937 rng_;
    std::uniform_real_distribution<double> quantityDist_;
    std::uniform_int_distribution<int> sideDist_;
};
