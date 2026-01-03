#pragma once
#include "i_order_generator.hpp"
#include "market_state.hpp"
#include <random>
#include <optional>
#include <sstream>

template <typename TOrder>
class RandomOrderGenerator : public IOrderGenerator<TOrder, RandomOrderGenerator<TOrder>> {
    
public:
    RandomOrderGenerator(MarketState &market, uint32_t maxQuantity, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();
private:
    uint32_t maxQuantity_;
    uint64_t orderId_ = 0;
    MarketState *marketState_;

    std::mt19937 rng_;
    std::uniform_int_distribution<uint32_t> quantityDist_;
    std::uniform_int_distribution<int> sideDist_;
    uint64_t sequence_number = 0;
};
