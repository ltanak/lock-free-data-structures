#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order.hpp"
#include "random_order_generator.hpp"
#include "collection_order_generator.hpp"

int main() {
    std::cout << "running code" << std::endl;
    RandomOrderGenerator<Order> orderGen = RandomOrderGenerator<Order>(10.0, 20.0, 50.0, 42);
    RandomOrderGenerator<Order> orderGen2 = RandomOrderGenerator<Order>(50.0, 100.0, 50.0, 42);
    std::vector<std::function<Order()>> gens {
        [&]() { return orderGen.generate() ;},
        [&]() { return orderGen2.generate();}
    };
    /**
     * type erased callable that returns order
     * [&] means we capture by reference
     * () means no parameters
     * if you use [] it means no outside variables are used (so e.g. we create the generators inside the lambda)
     */

    /**
     * TO IMPLEMENT NEXT
     * Need to update order generation model so that they read from the market state
     * Thread that controls the market state
     * Need to implement the stress testing and order testing code / generators
     * Once done need to then just print and output results and do a quick measure on the speed of transactions
     * Create "framework" / section where we will put the endpoint of the lock-free data structure to start getting ready for that
     */

     
    CollectionOrderGenerator<Order> collection(gens, 42);

    Order o = orderGen.generate();
    std::cout << "ID: " << o.order_id << ", Type: " << (o.type == OrderType::BUY ? "BUY" : "SELL") 
              << ", Price: " << o.price << ", Quantity: " << o.quantity << std::endl;
    std::cout << "Generating from collection:" << std::endl;
    for (int i = 0; i < 5; i++) {
        Order oc = collection.generate();
        std::cout << "ID: " << oc.order_id << ", Type: " << (oc.type == OrderType::BUY ? "BUY" : "SELL") 
                  << ", Price: " << oc.price << ", Quantity: " << oc.quantity << std::endl;
    }
    return 0;
}
