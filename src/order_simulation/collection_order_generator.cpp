#include "collection_order_generator.hpp"
#include <random>
#include <chrono>
#include <optional>
#include <functional>

template<typename TOrder>
CollectionOrderGenerator<TOrder>::CollectionOrderGenerator(std::vector<std::function<TOrder()>> generators, std::optional<uint32_t> seed){
    generatorsVector = std::move(generators);
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
    orderGenerators = std::uniform_int_distribution<int>(0, generatorsVector.size() - 1);
}

template<typename TOrder>
TOrder CollectionOrderGenerator<TOrder>::generateOrder() {
    orderId_++;
    int index = orderGenerators(rng_);
    TOrder order = generatorsVector[index](); // parentheses to call function
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp = std::chrono::high_resolution_clock::now();
    order.order_id = orderId_;
    order.timestamp = timestamp;
    return order;
}

#include "order.hpp"
template class CollectionOrderGenerator<Order>;