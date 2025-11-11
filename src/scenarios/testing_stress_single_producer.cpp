#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

/**
 * @brief Single producer stress test
 * Purpose of test is to benchmark the throughput of a single producer adding to a data structure
 * A single thread will be generating orders and pushing them to the data structure
 * 
 * @note Parameterised tests to be added in the future
 */

void singleProducerStressTest() {

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

        // datastructure.enqueue(oc);

        // gap

        // datastructure.deque();

    }

}