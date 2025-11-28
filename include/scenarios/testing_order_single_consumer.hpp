#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <queue>
#include <memory>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"
#include "scenarios/testing_order_single_consumer.hpp"
#include "scenarios/utils.hpp"

/**
 * @brief Single consumer order test
 * Purpose of test is to benchmark the ordering and correctness of orders being processed
 * Adds sequenced orders to a data structure from a single producer
 * Single thread will then deque orders from the data structure
 * 
 * @note Parameterised tests to be added in the future
 */

#define LIMIT 10

auto initialiseGenerators(MarketState &market) -> CollectionOrderGenerator<Order> {
    auto g1 = std::make_shared<RandomOrderGenerator<Order>>(market, 10, 42);
    auto g2 = std::make_shared<RandomOrderGenerator<Order>>(market, 100, 25);

    std::vector<std::function<Order()>> gens {
        [g1]() { return g1->generate() ;},
        [g2]() { return g2->generate();}
    };

    CollectionOrderGenerator<Order> collection(gens, 42);
    return collection;
}

template <typename DataStructure>
void singleConsumerOrderTest(DataStructure &structure) {
    std::cout << "Running single consumer order test: " << std::endl;

    std::queue<Order> ordersQueue;
    MarketState marketState;
    CollectionOrderGenerator<Order> collection = initialiseGenerators(marketState);

    // Generating orders
    std::cout << "Start?" << std::endl;
    for (int i = 0; i < LIMIT; ++i){
        Order o = collection.generate();
        o.sequence_number = i;
        ordersQueue.push(o); // emplace copies the actual structure, rather than doing it by pointer
    }
    std::cout << "Orders now going to be popped -> enqueued" << std::endl;


    // enqueueing onto the data structure
    while (!ordersQueue.empty()){
        Order o1 = ordersQueue.front();
        ordersQueue.pop();
        std::cout << o1 << std::endl;

        structure.enqueue(o1);
    }

    // dequeing from the data structure
    std::cout << "Now going to empty the data structure:" << std::endl;
    std::cout << structure.size() << std::endl;
    while (!structure.empty()){
        Order o;
        structure.dequeue(o);
        std::cout << o << std::endl;
    }
    std::cout << "Done!" << std::endl;

}