#pragma once
#include "i_order_generator.hpp"
#include <random>
#include <chrono>
#include <optional>
#include <functional>
#include <vector>

/**
 * @class CollectionOrderGenerator
 * @tparam TOrder
 * @brief Stores a vector of order generators, pseudorandmoly selects which one to generate from
 */
template <typename TOrder>
class CollectionOrderGenerator : public IOrderGenerator<TOrder, CollectionOrderGenerator<TOrder>> {

public:
    CollectionOrderGenerator(std::vector<std::function<TOrder()>> generators, std::optional<uint32_t> seed = std::nullopt);

    // generate function
    TOrder generateOrder();

    // to induce probabilistic bias into generator choosing
    void setWeights(const std::vector<double>& weights) noexcept;

private:
    uint64_t orderId_ = 0;
    std::mt19937 rng_;
    
    // using discrete_distribution
    std::discrete_distribution<int> generatorDist_;
    std::vector<std::function<TOrder()>> generatorsVector;
    std::vector<double> weights_;
};
