#pragma once

// regular includes
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <barrier>

// Order simulation includes
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Temporarily defining threads, will pass as parameter
#include "scenarios/test_inputs.hpp"

// custom utils
#include "utils/threads.hpp"
#include "utils/structs.hpp"
#include "utils/timing.hpp"

/**
 * @brief Stress Test
 * Purpose of test is to benchmark the throughput of multiple producers adding to a data structure
 * Multiple threads will be generating orders and pushing them to the data structure
 * Each producer thread will have its own order generation model
 * Multiple consumer threads will be dequeuing from the data structure at the same time
 * 
 * @note For SPSC data structures, inputs will be 1 producer, 1 consumer thread
 */

template <typename Wrapper>
void stressTest(Wrapper &wrapper, TestParams &params) {

    // consts from TestParams
    const uint64_t PRODUCERS = params.thread_count;
    const uint64_t CONSUMERS = params.thread_count;
    const uint64_t TOTAL_THREADS = PRODUCERS + CONSUMERS;
    const uint64_t TOTAL_ORDERS = params.total_orders;
    const uint64_t THREAD_LIMIT = params.thread_order_limit;
    const uint32_t SEED = params.seed;
    
    // warmup iterations
    constexpr uint32_t WARMUP_ITERS = 5000;

    // market state thread
    MarketState market_state;

    // after cache warming, before warmup
    std::barrier warmup_start_barrier(TOTAL_THREADS);
    // after warmup drain, before benchmark (empties queue)
    std::barrier drain_barrier(TOTAL_THREADS);
    // benchmark start
    std::barrier benchmark_barrier(TOTAL_THREADS);

    // atomic to signal producers have finished warmup enqueuing
    std::atomic<bool> warmup_producers_done{false};
    // atomic counter to drain the queue between warmup and benchmark
    std::atomic<uint64_t> warmup_enqueued{0};
    std::atomic<uint64_t> warmup_dequeued{0};

    // per-thread buffers
    std::vector<llogs::LatencyStore> thread_latencies(TOTAL_THREADS);
    for (auto &t_l: thread_latencies){
        t_l.enqueue_buffers = std::make_unique<uint64_t[]>(THREAD_LIMIT);
        t_l.dequeue_buffers = std::make_unique<uint64_t[]>(THREAD_LIMIT);
    }

    // producer threads for enqueuing
    std::vector<std::thread> producers;
    for (int i = 0; i < PRODUCERS; ++i) {
        int tid = wrapper.addEnqThread();
        producers.emplace_back(
            [&, tid, index = i, cpu = tid]() {
                RandomOrderGenerator<BenchmarkOrder> gen(market_state, 100 * (tid + 1), SEED + tid);
                lThread::pin_thread(cpu);
                auto *local_buffer = thread_latencies[index].enqueue_buffers.get();

                // touch the latency buffer pages
                for (uint64_t i = 0; i < THREAD_LIMIT; ++i){
                    local_buffer[i] = 0;
                }

                // enqueue real orders to warm data structure caches
                warmup_start_barrier.arrive_and_wait();
                
                for (uint64_t i = 0; i < WARMUP_ITERS; ++i){
                    BenchmarkOrder o = gen.generate();
                    wrapper.enqueueOrder(o, tid);
                    warmup_enqueued.fetch_add(1, std::memory_order_relaxed);
                }
                warmup_producers_done.store(true, std::memory_order_release);

                // wait for queue to be fully drained before starting benchmark
                drain_barrier.arrive_and_wait();

                // actual benchmark
                benchmark_barrier.arrive_and_wait();

                for (uint64_t i = 0; i < THREAD_LIMIT; ++i){
                    BenchmarkOrder o = gen.generate();

                    // start timestamp
                    uint64_t t0 = ltime::rdtsc_lfence();
                    // enqueue datastructure
                    wrapper.enqueueOrder(o, tid);
                    uint64_t t1 = ltime::rdtsc_lfence();
                    local_buffer[i] = t1 - t0;
                }
            }
        );
    }

    // consumer threads
    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        int tid = wrapper.addDeqThread();
        consumers.emplace_back(
            [&, tid, index = i, cpu = tid]() {
                BenchmarkOrder o;
                lThread::pin_thread(cpu);

                auto *local_buffer = thread_latencies[index].dequeue_buffers.get();
                
                // touch latency buffer
                for (uint64_t i = 0; i < THREAD_LIMIT; ++i){
                    local_buffer[i] = 0;
                }

                // dequeue alongside producers
                warmup_start_barrier.arrive_and_wait();
                
                // spin-dequeue: keep dequeuing until producers are done and queue is drained
                while (true) {
                    if (wrapper.dequeueLatency(o, tid)) {
                        warmup_dequeued.fetch_add(1, std::memory_order_relaxed);
                    }
                    // exit when producers are done and drained everything
                    if (warmup_producers_done.load(std::memory_order_acquire) &&
                        warmup_dequeued.load(std::memory_order_relaxed) >= warmup_enqueued.load(std::memory_order_relaxed)) {
                        break;
                    }
                }

                // queue empty signal ready
                drain_barrier.arrive_and_wait();
                
                // actual benchmark
                benchmark_barrier.arrive_and_wait();

                for (uint64_t count = 0; count < THREAD_LIMIT; ++count){

                    // start timestamp
                    uint64_t t0 = ltime::rdtsc_lfence();
                    wrapper.dequeueLatency(o, tid);
                    uint64_t t1 = ltime::rdtsc_lfence();
                    local_buffer[count] = t1 - t0;
                }
            }
        );
    }

    lThread::close(producers, consumers);

    std::vector<uint64_t> local_enqueues;
    std::vector<uint64_t> local_dequeues;
    local_enqueues.reserve(PRODUCERS * THREAD_LIMIT);
    local_dequeues.reserve(CONSUMERS * THREAD_LIMIT);

    // combines all results from each individual buffer
    for (auto &t_l: thread_latencies){
        std::copy(t_l.enqueue_buffers.get(), t_l.enqueue_buffers.get() + THREAD_LIMIT, std::back_inserter(local_enqueues));
        std::copy(t_l.dequeue_buffers.get(), t_l.dequeue_buffers.get() + THREAD_LIMIT, std::back_inserter(local_dequeues));
    }

    wrapper.setLatencyVectors(local_enqueues, local_dequeues);
    wrapper.processLatencies();
}
