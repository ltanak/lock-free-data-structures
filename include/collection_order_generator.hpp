#pragma once
#include "i_order_generator.hpp"
#include <random>
#include <chrono>
#include <optional>
#include <functional>

template <typename TOrder>
class CollectionOrderGenerator : public IOrderGenerator<TOrder, CollectionOrderGenerator<TOrder>> {

public:
    CollectionOrderGenerator(std::vector<std::function<TOrder()>> generators, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();
private:
    uint64_t orderId_;
    std::mt19937 rng_;
    std::uniform_int_distribution<int> orderGenerators;
    std::vector<std::function<TOrder()>> generatorsVector;
};