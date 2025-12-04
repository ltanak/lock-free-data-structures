#include <random>
#include <optional>
#include <atomic>

#include "order_simulation/order.hpp"
#include "data_structures/queues/regular_queue.hpp"
#include "scenarios/test_inputs.hpp"
#include "utils/timing.hpp"

#include "benchmarking/benchmark.hpp"

#include "utils/files.hpp"

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::BenchmarkWrapper(DataStructure &structure, TestParams &params)
: structure_(structure), localIndex_(params.thread_count, 0), TOTAL_ORDERS_(params.total_orders), NUM_THREADS_(params.thread_count), THREAD_LIMIT_(params.thread_order_limit)
{
    latencies_ = new uint64_t[params.total_orders];
    if (NUM_THREADS_ == 0 || THREAD_LIMIT_ == 0 || TOTAL_ORDERS_ == 0) {
        std::cerr << "Invalid params (zero)" << std::endl;
        std::abort();
    }
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
    uint64_t t0 = rdtscp_inline();
    // enqueue datastructure
    bool enqueued = structure_.enqueue(o);
    // end timer
    uint64_t t1 = rdtscp_inline();
    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    uint64_t idx = threadId * THREAD_LIMIT_ + localIndex_[threadId]++; // localIndex_ could be TLS or array
    latencies_[idx] = delta;
    return enqueued;
}
template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processLatencies(){
    double cycles_per_ns = measure_tsc_ghz();
    std::vector<double> ns_latencies;
    double sum = 0;

    for (size_t i = 0; i < TOTAL_ORDERS_; ++i) {
        double ns = latencies_[i] / cycles_per_ns;
        sum += ns;
        ns_latencies.push_back(ns);
    }
    sum /= TOTAL_ORDERS_;

    std::cout << "Avg Latency (ns): " << sum << std::endl;
    latencies::write_csv(ns_latencies);
    
}


template class BenchmarkWrapper<RegularQueue<Order>, Order>;