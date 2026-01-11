#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "order_simulation/benchmark_order.hpp"
#include <cstdint>

template<typename TOrder>
MCLockFreeQueue<TOrder>::MCLockFreeQueue() {}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::enqueueOrder(TOrder &order){
    if (mcQueue_.enqueue(order)){
        return true;
    }
    return false;
}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::dequeueOrder(TOrder &order){
    if (!mcQueue_.try_dequeue(order)){
        // will do a terminal logging error saying not possible
        return false;
    } else {
        return true;
    }
}

template<typename TOrder>
uint64_t MCLockFreeQueue<TOrder>::getSize(){
    return mcQueue_.size_approx();
}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::getFront(TOrder &order){
    if (auto *ptr = mcQueue_.peek()){
        order = *ptr;
        return true;
    }
    return false;
}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::isEmpty(){
    return mcQueue_.size_approx() == 0;
}

template class MCLockFreeQueue<BenchmarkOrder>;
