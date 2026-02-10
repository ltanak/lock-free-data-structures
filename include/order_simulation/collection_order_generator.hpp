#pragma once
#include "i_order_generator.hpp"
#include <random>
#include <chrono>
#include <optional>
#include <functional>
#include <vector>

template <typename TOrder>
class CollectionOrderGenerator : public IOrderGenerator<TOrder, CollectionOrderGenerator<TOrder>> {

public:
    CollectionOrderGenerator(std::vector<std::function<TOrder()>> generators, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();
    void setWeights(const std::vector<double>& weights) noexcept;

private:
    uint64_t orderId_ = 0;
    std::mt19937 rng_;
    // using discrete_distribution
    std::discrete_distribution<int> generatorDist_;
    std::vector<std::function<TOrder()>> generatorsVector;
    std::vector<double> weights_;
};
