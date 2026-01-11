#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"
#include "order_simulation/benchmark_order.hpp"
#include <cstdint>
#include <stdexcept>

#define NOT_IMPLEMENTED throw std::logic_error("Function not implemented.");

template<typename TOrder>
MCConcurrentQueue<TOrder>::MCConcurrentQueue() {}

template<typename TOrder>
bool MCConcurrentQueue<TOrder>::enqueueOrder(TOrder &order){
    if (mcMPMCQueue_.enqueue(order)){
        return true;
    }
    return false;
}

template<typename TOrder>
bool MCConcurrentQueue<TOrder>::dequeueOrder(TOrder &order){
    if (mcMPMCQueue_.try_dequeue(order)){
        return true;
    }
    return false;
}

template<typename TOrder>
uint64_t MCConcurrentQueue<TOrder>::getSize(){
    return mcMPMCQueue_.size_approx();
}

template<typename TOrder>
bool MCConcurrentQueue<TOrder>::getFront(TOrder &order){
    // TOrder *order = mcMPMCQueue_.
    // there isn't an implementation for peek, it does it through deque
    NOT_IMPLEMENTED
    return false;
    // TOrder order;
    // bool success = mcMPMCQueue_.try_dequeue(order);
    // if (!success){
    //     // error message here
    // }
    // size_--;
    // return order;
}

template<typename TOrder>
bool MCConcurrentQueue<TOrder>::isEmpty(){
    return mcMPMCQueue_.size_approx() == 0;
}

template class MCConcurrentQueue<BenchmarkOrder>;
