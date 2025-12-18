#include <random>
#include <optional>
#include <atomic>

#include "order_simulation/order.hpp"
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"
#include "scenarios/test_inputs.hpp"
#include "utils/timing.hpp"

#include "benchmarking/benchmark.hpp"

#include "utils/files.hpp"

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::BenchmarkWrapper(DataStructure &structure, TestParams &params)
: structure_(structure), localIndexEnq_(params.thread_count, 0), localIndexDeq_(params.thread_count, 0), TOTAL_ORDERS_(params.total_orders), NUM_THREADS_(params.thread_count), THREAD_LIMIT_(params.thread_order_limit)
{
    latencies_enqueue = new uint64_t[params.total_orders];
    latencies_dequeue = new uint64_t[params.total_orders];
    if (NUM_THREADS_ == 0 || THREAD_LIMIT_ == 0 || TOTAL_ORDERS_ == 0) {
        std::cerr << "Invalid params (zero)" << std::endl;
        std::abort();
    }
}

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::~BenchmarkWrapper(){
    delete[] latencies_enqueue;
    delete[] latencies_dequeue;
}

template<typename DataStructure, typename TOrder>
int BenchmarkWrapper<DataStructure, TOrder>::addThread(){
    return enqueueThreadId_.fetch_add(1, std::memory_order_relaxed);
}

template<typename DataStructure, typename TOrder>
int BenchmarkWrapper<DataStructure, TOrder>::addDequeueThread(){
    return dequeueThreadId_.fetch_add(1, std::memory_order_relaxed);
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::enqueue_order(TOrder &o, int threadId) {
    // start timer
    uint64_t t0 = lTime::rdtscp_inline();
    // enqueue datastructure
    bool enqueued = structure_.enqueue(o);
    // end timer
    uint64_t t1 = lTime::rdtscp_inline();
    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    uint64_t idx = threadId * THREAD_LIMIT_ + localIndexEnq_[threadId]++; // per-producer index
    latencies_enqueue[idx] = delta;
    return enqueued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::dequeue_order(TOrder &o, int threadId) {
    // start timer
    uint64_t t0 = lTime::rdtscp_inline();
    // dequeue datastructure
    bool dequeued = structure_.dequeue(o);
    // end timer
    uint64_t t1 = lTime::rdtscp_inline();
    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    uint64_t idx = threadId * THREAD_LIMIT_ + localIndexDeq_[threadId]++; // per-consumer index
    latencies_dequeue[idx] = delta;
    return dequeued;
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processLatencies(){
    double cycles_per_ns = lTime::measure_tsc_ghz();
    std::vector<double> e_ns_latencies;
    std::vector<double> d_ns_latencies;
    double sumEnq = 0;
    double sumDeq = 0;

    for (size_t i = 0; i < TOTAL_ORDERS_; ++i) {
        double e_ns = latencies_enqueue[i] / cycles_per_ns;
        sumEnq += e_ns;
        e_ns_latencies.push_back(e_ns);

        double d_ns = latencies_dequeue[i] / cycles_per_ns;
        sumDeq += d_ns;
        d_ns_latencies.push_back(d_ns);
    }
    sumEnq /= TOTAL_ORDERS_;
    sumDeq /= TOTAL_ORDERS_;

    std::cout << "Avg Enqueue Latency (ns): " << sumEnq << std::endl;
    std::cout << "Avg Dequeue Latency (ns): " << sumDeq << std::endl;
    latencies::write_csv_latencies(e_ns_latencies, d_ns_latencies);
}

template class BenchmarkWrapper<RegularQueue<Order>, Order>;
template class BenchmarkWrapper<MCLockFreeQueue<Order>, Order>;
template class BenchmarkWrapper<MCConcurrentQueue<Order>, Order>;