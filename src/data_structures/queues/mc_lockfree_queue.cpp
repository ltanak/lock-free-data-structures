#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "order_simulation/order.hpp"

template<typename TOrder>
MCLockFreeQueue<TOrder>::MCLockFreeQueue(): size_(0) {}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::enqueueOrder(TOrder &order){
    // implement adding to the queue
    return false;
}

template<typename TOrder>
void MCLockFreeQueue<TOrder>::dequeueOrder(){
    // implement dequeing
    return;
}

template<typename TOrder>
uint64_t MCLockFreeQueue<TOrder>::getSize(){
    // implement size checking
    return -1;
}

template<typename TOrder>
TOrder MCLockFreeQueue<TOrder>::getFront(){
    // implement
    return {};
}

template class MCLockFreeQueue<Order>;