#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <queue>
#include <thread>
#include <memory>
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Input parameters
#include "scenarios/test_inputs.hpp"

/**
 * @brief Order test
 * Purpose of test is to benchmark the ordering and correctness of orders being processed
 * Adds sequenced orders to a data structure from a single producer
 * Multiple consumers will then deque orders from the data structure
 * 
 * @note Parameterised tests to be added in the future, and will eventually pass through a price-time priority exchange
 */

auto initialiseGeneratorsOrder(MarketState &market, const uint32_t SEED) -> CollectionOrderGenerator<BenchmarkOrder> {
    auto g1 = std::make_shared<RandomOrderGenerator<BenchmarkOrder>>(market, 10, SEED);
    auto g2 = std::make_shared<RandomOrderGenerator<BenchmarkOrder>>(market, 100, SEED + 1);

    std::vector<std::function<BenchmarkOrder()>> gens {
        [g1]() { return g1->generate();},
        [g2]() { return g2->generate();}
    };

    CollectionOrderGenerator<BenchmarkOrder> collection(gens, 42);
    return collection;
}

template <typename Wrapper>
void orderTest(Wrapper &wrapper, TestParams &params) {
    const uint64_t CONSUMERS = params.thread_count;
    const uint64_t TOTAL_ORDERS = params.total_orders;
    const uint64_t THREAD_LIMIT = params.thread_order_limit;
    const uint32_t SEED = params.seed;

    std::queue<BenchmarkOrder> ordersQueue;
    MarketState marketState;

    CollectionOrderGenerator<BenchmarkOrder> collection = initialiseGeneratorsOrder(marketState, SEED);
    CollectionOrderGenerator<BenchmarkOrder> checkGen = initialiseGeneratorsOrder(marketState, SEED);

    for (int i = 0; i < TOTAL_ORDERS; ++i){
        BenchmarkOrder o = collection.generate();
        o.sequence_number = i + 1;
        ordersQueue.emplace(o); // emplace copies the actual thing, rather than doing it by pointer
    }

    // enqueueing onto the data structure
    while (!ordersQueue.empty()){
        BenchmarkOrder o1 = ordersQueue.front();
        ordersQueue.pop();
        // std::cout << o1 << std::endl;

        wrapper.enqueue_order(o1, 0);
    }

    // dequeing from the data structure
    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        int tid = wrapper.addDequeueThread();
        consumers.emplace_back(
            [&, tid]() {
                BenchmarkOrder o;
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
    CollectionOrderGenerator<BenchmarkOrder> resetGen = initialiseGeneratorsOrder(resetMarketstate, SEED);

    wrapper.processOrders(resetGen);
    std::cout << "Ordering test completed with " << CONSUMERS << " consumers.\n";
}
