#include "order_generator.hpp"
#include <random>
#include <chrono>

template <typename TOrder>
class RandomOrderGenerator : public OrderGenInterface<TOrder, RandomOrderGenerator<TOrder>> {
    
    // this class generates random orders
    // will be using seeded random number generator for reproducibility
};