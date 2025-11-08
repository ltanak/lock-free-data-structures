#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

void singleProducerStressTest() {
    // Placeholder for single producer stress test implementation
    // This function would typically create a single producer thread
    // that generates orders at a high rate to test the system's performance
    // under load. The implementation details would depend on the rest
    // of the order simulation framework.

    MarketState marketState;
    RandomOrderGenerator<Order> marketGen = RandomOrderGenerator<Order>(marketState, 100, 42);
    Order o;
    for (int i = 0; i <= 5; ++i){
        o = marketGen.generate();
        std::cout << "ID: " << o.order_id << ", Type: " << (o.type == OrderType::BUY ? "BUY" : "SELL") 
                  << ", Price: " << o.price << ", Quantity: " << o.quantity << std::endl;
    }

    std::cout << "Collection Testing: \n";
    RandomOrderGenerator<Order> g1 = RandomOrderGenerator<Order>(marketState, 10, 42);
    RandomOrderGenerator<Order> g2 = RandomOrderGenerator<Order>(marketState, 100, 25);
    Order o1 = g1.generate();
    Order o2 = g2.generate();
    std::cout << "ID: " << o1.order_id << ", Type: " << (o1.type == OrderType::BUY ? "BUY" : "SELL") 
    << ", Price: " << o1.price << ", Quantity: " << o1.quantity << std::endl;
    std::cout << "ID: " << o2.order_id << ", Type: " << (o2.type == OrderType::BUY ? "BUY" : "SELL") 
    << ", Price: " << o2.price << ", Quantity: " << o2.quantity << std::endl;
    
    /**
     * type erased callable that returns order
     * [&] means we capture by reference
     * () means no parameters
     * if you use [] it means no outside variables are used (so e.g. we create the generators inside the lambda)
     */
    std::vector<std::function<Order()>> gens {
        [&]() { return g1.generate() ;},
        [&]() { return g2.generate();}
    };

    CollectionOrderGenerator<Order> collection(gens, 42);
    std::cout << "Generating from collection:" << std::endl;
    for (int i = 0; i < 1000; i++) {
        Order oc = collection.generate();
        std::cout << "ID: " << oc.order_id << ", Type: " << (oc.type == OrderType::BUY ? "BUY" : "SELL") 
                  << ", Price: " << oc.price << ", Quantity: " << oc.quantity << std::endl;
    }

}