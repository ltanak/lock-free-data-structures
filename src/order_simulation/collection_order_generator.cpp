#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "utils/timing.hpp"

template<typename TOrder>
CollectionOrderGenerator<TOrder>::CollectionOrderGenerator(std::vector<std::function<TOrder()>> generators, std::optional<uint32_t> seed){
    generatorsVector = std::move(generators);
    // Default uniform weights
    weights_.resize(generatorsVector.size(), 1.0 / generatorsVector.size());
    generatorDist_ = std::discrete_distribution<int>(weights_.begin(), weights_.end());
    
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
}

template<typename TOrder>
void CollectionOrderGenerator<TOrder>::setWeights(const std::vector<double>& weights) noexcept {
    if (weights.size() == generatorsVector.size()) {
        weights_ = weights;
        generatorDist_ = std::discrete_distribution<int>(weights_.begin(), weights_.end());
    }
}

template<typename TOrder>
TOrder CollectionOrderGenerator<TOrder>::generateOrder() {
    orderId_++;
    int index = generatorDist_(rng_);
    TOrder order = generatorsVector[index](); // calls the order generator function
    order.order_id = orderId_;
    order.timestamp = ltime::rdtsc_lfence();
    return order;
}

template class CollectionOrderGenerator<BenchmarkOrder>;
