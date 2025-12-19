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

// Input parameters
#include "scenarios/test_inputs.hpp"

// POSSIBLE REFACTOR - MERGE BOTH ORDER TESTS AND JUST SET THE SINGLE ONE TO ONE THREAD - WILL HAVE THE EXACT SAME EFFECT AND WOULD
// SAVE LINES OF CODE

/**
 * @brief Order test
 * Purpose of test is to benchmark the ordering and correctness of orders being processed
 * Adds sequenced orders to a data structure from a single producer
 * Multiple consumers will then deque orders from the data structure
 * 
 * @note Parameterised tests to be added in the future
 */

auto initialiseGeneratorsOrder(MarketState &market) -> CollectionOrderGenerator<Order> {
    auto g1 = std::make_shared<RandomOrderGenerator<Order>>(market, 10, 42);
    auto g2 = std::make_shared<RandomOrderGenerator<Order>>(market, 100, 25);

    std::vector<std::function<Order()>> gens {
        [g1]() { return g1->generate();},
        [g2]() { return g2->generate();}
    };

    CollectionOrderGenerator<Order> collection(gens, 42);
    return collection;
}

template <typename Wrapper>
void orderTest(Wrapper &wrapper, TestParams &params) {
    const uint64_t CONSUMERS = params.thread_count;
    const uint64_t TOTAL_ORDERS = params.total_orders;
    const uint64_t THREAD_LIMIT = params.thread_order_limit;

    std::queue<Order> ordersQueue;
    MarketState marketState;

    CollectionOrderGenerator<Order> collection = initialiseGeneratorsOrder(marketState);
    CollectionOrderGenerator<Order> checkGen = initialiseGeneratorsOrder(marketState);

    for (int i = 0; i < TOTAL_ORDERS; ++i){
        Order o = collection.generate();
        o.sequence_number = i + 1;
        ordersQueue.emplace(o); // emplace copies the actual thing, rather than doing it by pointer
    }

    // enqueueing onto the data structure
    while (!ordersQueue.empty()){
        Order o1 = ordersQueue.front();
        ordersQueue.pop();
        // std::cout << o1 << std::endl;

        wrapper.enqueue_order(o1, 0);
    }

    // dequeing from the data structure
    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        int tid = wrapper.addDequeueThread();
        consumers.emplace_back(
            [&, i, tid]() {
                Order o;
                uint64_t count = 0;
                while (true){
                    if (count >= THREAD_LIMIT) break;
                    ++count;
                    wrapper.dequeue_ordering(o, tid);
                }
            }
        );
    }

    lThread::close(consumers);

    // as order generators are seeded, this will provide the exact same sequence of transactions to compare to
    MarketState resetMarketstate;
    CollectionOrderGenerator<Order> resetGen = initialiseGeneratorsOrder(resetMarketstate);
    // for (int i = 0; i < TOTAL_ORDERS; ++i){
    //     Order anotherCheck = afterCheck.generate();
    //     std::cout << "Final check order: " << anotherCheck << std::endl;
    // }
    wrapper.processOrders(resetGen);
    std::cout << "Ordering test completed with " << CONSUMERS << " consumers.\n";
}
