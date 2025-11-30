#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

template<typename DataStructure>
void singleProducerStressTest(DataStructure &structure) {
    std::cout << "Running single producer stress test: " << std::endl;

    MarketState marketState;
    // would need to start the thread for the market state here
    RandomOrderGenerator<Order> g1 = RandomOrderGenerator<Order>(marketState, 10, 42);
    RandomOrderGenerator<Order> g2 = RandomOrderGenerator<Order>(marketState, 100, 25);

    std::vector<std::function<Order()>> gens {
        [&]() { return g1.generate() ;},
        [&]() { return g2.generate();}
    };

    CollectionOrderGenerator<Order> collection(gens, 42);
    std::cout << "Generating from collection:" << std::endl;
    for (int i = 0; i < 10000; i++) {
        Order oc = collection.generate(); // generates an order from the collection

        // THIS IS WHERE WE WILL BE PERFORMING ENQUEUE FOR THE DATA STRUCTURE BEING TESTED

        structure.enqueue(oc);

        // gap

        // datastructure.deque();

    }
    std::cout << "Done" << std::endl;

    std::cout << "Output some elemnts" << std::endl;
    Order x;
    structure.dequeue(x);
    std::cout << x << std::endl;
}