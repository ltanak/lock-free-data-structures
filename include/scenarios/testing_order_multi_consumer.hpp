#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <queue>
#include <thread>
#include <memory>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"
#include "scenarios/testing_order_multi_consumer.hpp"

// POSSIBLE REFACTOR - MERGE BOTH ORDER TESTS AND JUST SET THE SINGLE ONE TO ONE THREAD - WILL HAVE THE EXACT SAME EFFECT AND WOULD
// SAVE LINES OF CODE

/**
 * @brief Multi consumer order test
 * Purpose of test is to benchmark the ordering and correctness of orders being processed
 * Adds sequenced orders to a data structure from a single producer
 * Multiple consumers will then deque orders from the data structure
 * 
 * @note Parameterised tests to be added in the future
 */

#define CONSUMERS 1

auto initialiseGeneratorsOrder(MarketState &market) -> CollectionOrderGenerator<Order> {
    auto g1 = std::make_shared<RandomOrderGenerator<Order>>(market, 10, 42);
    auto g2 = std::make_shared<RandomOrderGenerator<Order>>(market, 100, 25);

    std::vector<std::function<Order()>> gens {
        [g1]() { return g1->generate() ;},
        [g2]() { return g2->generate();}
    };

    CollectionOrderGenerator<Order> collection(gens, 42);
    return collection;
}

auto closeThreads(std::vector<std::thread> &consumers) -> void {
    // may require additional logic in the future
    for (auto& cons: consumers){
        cons.join();
    }
}

template <typename DataStructure>
void multiConsumerOrderTest(DataStructure &structure) {
    std::cout << "Running multi consumer order test: " << std::endl;


    // has to be expr as later when scripting we will add parameters for setting threads
    // constexpr CONSUMERS = 4;
    std::atomic<bool> running{true};

    std::queue<Order> ordersQueue;
    MarketState marketState;

    CollectionOrderGenerator<Order> collection = initialiseGeneratorsOrder(marketState);
    for (int i = 0; i < 20; ++i){
        Order o = collection.generate();
        o.sequence_number = i;
        ordersQueue.emplace(o); // emplace copies the actual thing, rather than doing it by pointer
    }
    std::cout << "Orders now going to be popped -> enqueued" << std::endl;


    // enqueueing onto the data structure
    while (!ordersQueue.empty()){
        Order o1 = ordersQueue.front(); // CHECK THIS - NEED TO MAKE SURE IT ACTUALLY 
        ordersQueue.pop();
        std::cout << o1 << std::endl;

        structure.enqueue(o1);
    }


    // dequeing from the data structure
    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        consumers.emplace_back(
            [&, i]() {
                Order o;
                while (running.load(std::memory_order_relaxed)){
                    structure.dequeue(o);
                    // can also print out results here
                }
            }
        );
    }

    std::cout << "Running the threads" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Timer done" << std::endl;

    running.store(false);

    closeThreads(consumers);
}