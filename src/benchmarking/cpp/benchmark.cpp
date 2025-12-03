#include <random>
#include <optional>
#include <atomic>

#include "order_simulation/order.hpp"
#include "data_structures/queues/regular_queue.hpp"
#include "scenarios/test_inputs.hpp"
#include "utils/timing.hpp"

#include "benchmarking/benchmark.hpp"

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::BenchmarkWrapper(DataStructure &structure, TestParams &params)
: structure_(structure), localIndex_(NUM_THREADS_, 0), TOTAL_ORDERS_(params.total_orders), NUM_THREADS_(params.thread_count), THREAD_LIMIT_(params.thread_order_limit)
{
    latencies_ = new uint64_t[TOTAL_ORDERS_];
}

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::~BenchmarkWrapper(){
    delete[] latencies_;
}

template<typename DataStructure, typename TOrder>
int BenchmarkWrapper<DataStructure, TOrder>::addThread(){
    return nextThreadId_.fetch_add(1, std::memory_order_relaxed);
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::enqueue_order(TOrder &o, int threadId) {
    // start timer
    // std::cout << "Start" << std::endl;
    uint64_t t0 = rdtscp_inline();
    // enqueue datastructure
    bool enqueued = structure_.enqueue(o);
    // end timer
    uint64_t t1 = rdtscp_inline();

    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    auto idx = (NUM_THREADS_ * threadId) + localIndex_[threadId]++; // localIndex_ could be TLS or array
    latencies_[idx] = delta;
    // std::cout << "End" << std::endl;
    return enqueued;
}
template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processLatencies(){
    u_int64_t sum = 0;
    for (size_t i = 0; i < TOTAL_ORDERS_; ++i) {
        std::cout << "Latency: " << latencies_[i] << std::endl;
        sum += latencies_[i];
    }
    sum /= NUM_THREADS_;

    std::cout << "Avg Latency: " << sum << std::endl;
}


template class BenchmarkWrapper<RegularQueue<Order>, Order>;