#pragma once

#include "i_order_generator.hpp"
#include "market_state.hpp"
#include <random>
#include <chrono>
#include <optional>
#include <functional>

/**
 * Reads the marketstate thread, then will quote around the price
 */

template<typename TOrder>
class MarketMakerGenerator : IOrderGenerator<TOrder, MarketMakerGenerator<TOrder>> {

public:
    MarketMakerGenerator(MarketState &market, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();
private:

    uint64_t orderId_ = 0;
    MarketState *marketState_;

    std::mt19937 rng_;
    std::uniform_int_distribution<uint32_t> quantityDist_;
    std::uniform_int_distribution<int> sideDist_;
    uint64_t sequence_number = 0;
};