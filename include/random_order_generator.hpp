#pragma once
#include "i_order_generator.hpp"
#include <random>
#include <optional>

template <typename TOrder>
class RandomOrderGenerator : public IOrderGenerator<TOrder, RandomOrderGenerator<TOrder>> {
    
public:
    RandomOrderGenerator(double minPrice, double maxPrice, double maxQuantity, std::optional<uint32_t> seed = std::nullopt);
    TOrder generateOrder();
private:
    double minPrice_;
    double maxPrice_;
    double maxQuantity_;
    uint64_t orderId_ = 0;

    std::mt19937 rng_;
    std::uniform_real_distribution<double> priceDist_;
    std::uniform_real_distribution<double> quantityDist_;
    std::uniform_int_distribution<int> sideDist_;
};