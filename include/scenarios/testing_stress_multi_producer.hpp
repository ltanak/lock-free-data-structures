#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

#define PRODUCERS 1
#define CONSUMERS 1


/**
 * @brief Multi producer stress test
 * Purpose of test is to benchmark the throughput of multiple producers adding to a data structure
 * Multiple threads will be generating orders and pushing them to the data structure
 * Each producer thread will have its own order generation model
 * Multiple consumer threads will be dequeuing from the data structure at the same time
 * 
 * @note Parameterised tests to be added in the future
 */

void closeThreads(std::vector<std::thread> &producers, std::vector<std::thread> &consumers){
    for (auto& prod: producers){
        prod.join();
    }
    for (auto& cons: consumers){
        cons.join();
    }
}

template <typename DataStructure>
void multiProducerStressTest(DataStructure &structure) {
    std::cout << "Running multi producer stress test: " << std::endl;

    std::atomic<bool> running{true};
    std::vector<uint64_t> counts(PRODUCERS, 0);
    MarketState marketState;

    std::vector<std::thread> producers;
    for (int i = 0; i < PRODUCERS; ++i) {
        producers.emplace_back(
            [&, i]() {
                uint64_t count = 0;
                RandomOrderGenerator<Order> gen(marketState, 100 * (i + 1), 100 + i);
                while (running.load(std::memory_order_relaxed)){
                    Order o = gen.generate();
                    ++count;
                    structure.enqueue(o);
                    // will print out result to test it works
                }
                counts[i] = count; 
            }
        );
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        consumers.emplace_back(
            [&, i]() {
                Order o;
                while (running.load(std::memory_order_relaxed)){
                    structure.dequeue();
                    // can also print out results here
                }
            }
        );
    }

    std::cout << "Running the threads" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "Timer done" << std::endl;

    running.store(false);
    closeThreads(producers, consumers);

    std::cout << "Counts: ";
    for (auto& c: counts){
        std::cout << c << ", ";
    }
    std::cout << std::endl;

    std::cout << "Stress test completed with " << PRODUCERS << " producers and " << CONSUMERS << " consumers.\n";
}
