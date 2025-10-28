#pragma once
#include "order_generator.hpp"
#include "order.hpp"
#include <random>
#include <chrono>

template <typename TOrder>
class RandomOrderGenerator : public OrderGenInterface<TOrder, RandomOrderGenerator<TOrder>> {
    
    // this class generates random orders
    double minPrice_;
    double maxPrice_;
    double maxQuantity_;

    uint64_t orderId_ = 0;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> priceDist_;
    std::uniform_real_distribution<double> quantityDist_;
    std::uniform_int_distribution<int> sideDist_;
    // will be using seeded random number generator for reproducibility
public:
    RandomOrderGenerator(double minPrice, double maxPrice, double maxQuantity, std::optional<uint32_t> seed = std::nullopt) {
        minPrice_ = minPrice;
        maxPrice_ = maxPrice;
        maxQuantity_ = maxQuantity;
        if (seed.has_value()) {
            rng_ = std::mt19937(seed.value());
        } else {
            std::random_device rd;
            rng_ = std::mt19937(rd());
        }
        priceDist_ = std::uniform_real_distribution<double>(minPrice_, maxPrice_);
        quantityDist_ = std::uniform_real_distribution<double>(1.0, maxQuantity_);
        sideDist_ = std::uniform_int_distribution<int>(0, 1);
    } // 0 for buy, 1 for sell

    // uint64_t order_id;
    // OrderType type;
    // double price;
    // uint32_t quantity;
    // std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;

    TOrder generateOrder() {
        orderId_++;
        OrderType orderType = (sideDist_(rng_) == 0) ? OrderType::BUY : OrderType::SELL;
        double price = priceDist_(rng_);
        double quantity = quantityDist_(rng_);
        std::chrono::time_point<std::chrono::high_resolution_clock> timestamp = std::chrono::high_resolution_clock::now();
        return TOrder{orderId_, orderType, price, static_cast<uint32_t>(quantity), timestamp};
    }
};