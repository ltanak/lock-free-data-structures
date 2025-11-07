#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order.hpp"
#include "random_order_generator.hpp"
#include "collection_order_generator.hpp"
#include "market_state.hpp"

int main() {
    std::cout << "running code" << std::endl;

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

    // RandomOrderGenerator<Order> orderGen = RandomOrderGenerator<Order>(10.0, 20.0, 50.0, 42);
    // RandomOrderGenerator<Order> orderGen2 = RandomOrderGenerator<Order>(50.0, 100.0, 50.0, 42);
    // std::vector<std::function<Order()>> gens {
    //     [&]() { return orderGen.generate() ;},
    //     [&]() { return orderGen2.generate();}
    // };
    /**
     * type erased callable that returns order
     * [&] means we capture by reference
     * () means no parameters
     * if you use [] it means no outside variables are used (so e.g. we create the generators inside the lambda)
     */

    /**
     * TO IMPLEMENT NEXT
     * DONE - Need to update order generation model so that they read from the market state
     * Thread that controls the market state
     * Need to implement the stress testing and order testing code / generators
     * Once done need to then just print and output results and do a quick measure on the speed of transactions
     * Create "framework" / section where we will put the endpoint of the lock-free data structure to start getting ready for that
     */

    // CollectionOrderGenerator<Order> collection(gens, 42);

    // Order o = orderGen.generate();
    // std::cout << "ID: " << o.order_id << ", Type: " << (o.type == OrderType::BUY ? "BUY" : "SELL") 
    //           << ", Price: " << o.price << ", Quantity: " << o.quantity << std::endl;
    // std::cout << "Generating from collection:" << std::endl;
    // for (int i = 0; i < 5; i++) {
    //     Order oc = collection.generate();
    //     std::cout << "ID: " << oc.order_id << ", Type: " << (oc.type == OrderType::BUY ? "BUY" : "SELL") 
    //               << ", Price: " << oc.price << ", Quantity: " << oc.quantity << std::endl;
    // }
    return 0;
}
