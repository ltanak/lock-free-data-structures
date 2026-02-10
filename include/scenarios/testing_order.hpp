#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <queue>
#include <thread>
#include <memory>
#include <barrier>
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/market_maker_generator.hpp"
#include "order_simulation/momentum_generator.hpp"
#include "order_simulation/mean_revert_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Input parameters
#include "scenarios/test_inputs.hpp"

// Utils
#include "utils/structs.hpp"
#include "utils/timing.hpp"

/**
 * @brief Order test
 * Purpose of test is to benchmark the ordering and correctness of orders being processed
 * Adds sequenced orders to a data structure from a single producer
 * Multiple consumers will then deque orders from the data structure
 */

auto initialiseGeneratorsOrder(MarketState &market, const uint32_t SEED) -> CollectionOrderGenerator<BenchmarkOrder> {
    auto random1 = std::make_shared<RandomOrderGenerator<BenchmarkOrder>>(market, 20, SEED);
    auto random2 = std::make_shared<RandomOrderGenerator<BenchmarkOrder>>(market, 80, SEED + 1);
    auto market_maker = std::make_shared<MarketMakerGenerator<BenchmarkOrder>>(market, SEED + 2);
    auto momentum = std::make_shared<MomentumGenerator<BenchmarkOrder>>(market, SEED + 3);
    auto mean_revert = std::make_shared<MeanRevertGenerator<BenchmarkOrder>>(market, 100.0, SEED + 4);

    std::vector<std::function<BenchmarkOrder()>> gens {
        [random1]() { return random1->generateOrder();},
        [random2]() { return random2->generateOrder();},
        [market_maker]() { return market_maker->generateOrder();},
        [momentum]() { return momentum->generateOrder();},
        [mean_revert]() { return mean_revert->generateOrder();}
    };

    // Weights: 10% retail small, 10% retail large, 40% market maker, 20% momentum, 20% mean revert
    CollectionOrderGenerator<BenchmarkOrder> collection(gens, 42);
    collection.setWeights({0.10, 0.10, 0.40, 0.20, 0.20});
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

    std::barrier benchmark_barrier(CONSUMERS);

    std::vector<llogs::OrderingStore> thread_ordering(CONSUMERS);
    for (auto &t_o: thread_ordering){
        t_o.sequence_buffers = std::make_unique<uint64_t[]>(THREAD_LIMIT);
        t_o.timestamp_buffers = std::make_unique<uint64_t[]>(THREAD_LIMIT);
    }

    for (int i = 0; i < TOTAL_ORDERS; ++i){
        // Update market price with drift and volatility
        marketState.updatePrice();
        
        BenchmarkOrder o = collection.generate();
        o.sequence_number = i + 1;
        ordersQueue.emplace(o); // emplace copies the actual thing, rather than doing it by pointer
        
        // event triggering every 200 orders (scales well for 1k-100k)
        if ((i + 1) % 200 == 0) {
            marketState.checkAndApplyEvent(i + 1);
        }
    }

    // enqueueing onto the data structure
    while (!ordersQueue.empty()){
        BenchmarkOrder o1 = ordersQueue.front();
        ordersQueue.pop();
        wrapper.enqueueOrder(o1, 0);
    }

    // dequeing from the data structure
    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        int tid = wrapper.addDeqThread();
        consumers.emplace_back(
            [&, tid, index = i, cpu = tid]() {
                BenchmarkOrder o;
                lThread::pin_thread(cpu);
                auto *sequence_buffer = thread_ordering[index].sequence_buffers.get();
                auto *timestamp_buffer = thread_ordering[index].timestamp_buffers.get();
                // uint64_t count = 0;
                // while (true){
                //     if (count >= THREAD_LIMIT) break;
                //     ++count;
                //     wrapper.dequeueOrdering(o, tid);
                // }

                // barrier for synchronisation
                benchmark_barrier.arrive_and_wait();

                for (uint64_t i = 0; i < THREAD_LIMIT; ++i){              
                    wrapper.dequeueOrdering(o, tid);
                    timestamp_buffer[i] = ltime::rdtsc_lfence();
                    sequence_buffer[i] = o.sequence_number;
                }
            }
        );
    }

    lThread::close(consumers);

    std::vector<uint64_t> local_timestamps;
    std::vector<uint64_t> local_sequences;
    local_timestamps.reserve(CONSUMERS * THREAD_LIMIT);
    local_sequences.reserve(CONSUMERS * THREAD_LIMIT);

    for (auto &t_o: thread_ordering){
        std::copy(t_o.timestamp_buffers.get(), t_o.timestamp_buffers.get() + THREAD_LIMIT, std::back_inserter(local_timestamps));
        std::copy(t_o.sequence_buffers.get(), t_o.sequence_buffers.get() + THREAD_LIMIT, std::back_inserter(local_sequences));
    }

    wrapper.setOrderingVectors(local_timestamps, local_sequences);
    // as order generators are seeded, this will provide the exact same sequence of transactions to compare to
    MarketState resetMarketstate;
    CollectionOrderGenerator<BenchmarkOrder> resetGen = initialiseGeneratorsOrder(resetMarketstate, SEED);

    wrapper.processOrders(resetGen, resetMarketstate);
    std::cout << "Ordering test completed with " << CONSUMERS << " consumers.\n";
}
