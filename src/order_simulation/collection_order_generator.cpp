#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "utils/timing.hpp"

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
    // uint64_t id = orderId_.fetch_add(1, std::memory_order_relaxed);
    orderId_++;
    int index = orderGenerators(rng_);
    TOrder order = generatorsVector[index](); // calls the order generator function
    order.order_id = orderId_;
    order.timestamp = lTime::rdtscp_inline();
    return order;
}

template class CollectionOrderGenerator<BenchmarkOrder>;
