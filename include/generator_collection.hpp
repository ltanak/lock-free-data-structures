#pragma once

#pragma once
#include "order_generator.hpp"
#include "order.hpp"
#include <random>
#include <chrono>

template <typename TOrder>
class GeneratorCollection : public OrderGenInterface<TOrder, GeneratorCollection<TOrder>> {

    uint64_t orderId_ = 0;
    std::mt19937 rng_;
    std::uniform_int_distribution<int> orderGenerators;
    std::vector<std::function<TOrder()>> generatorsVector;

public:
    GeneratorCollection(std::vector<std::function<TOrder()>> generators, std::optional<uint32_t> seed = std::nullopt) {
        generatorsVector = std::move(generators);
        if (seed.has_value()) {
            rng_ = std::mt19937(seed.value());
        } else {
            std::random_device rd;
            rng_ = std::mt19937(rd());
        }
        orderGenerators = std::uniform_int_distribution<int>(0, generatorsVector.size() - 1);
    } // 0 for buy, 1 for sell

    // uint64_t order_id;
    // OrderType type;
    // double price;
    // uint32_t quantity;
    // std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;

    TOrder generateOrder() {
        orderId_++;
        int index = orderGenerators(rng_);
        TOrder order = generatorsVector[index](); // parentheses to call function
        std::chrono::time_point<std::chrono::high_resolution_clock> timestamp = std::chrono::high_resolution_clock::now();
        order.order_id = orderId_;
        order.timestamp = timestamp;
        return order;
    }
};