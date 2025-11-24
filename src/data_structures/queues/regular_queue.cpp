#include <queue>
#include <mutex>
#include <random>
#include <chrono>
#include <optional>
#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/regular_queue.hpp"

template<typename TOrder>
RegularQueue<TOrder>::RegularQueue(): size(0) {}

template<typename TOrder>
bool RegularQueue<TOrder>::enqueueOrder(TOrder &order){
    queue_.push(order)
}

template<typename TOrder>
TOrder RegularQueue<TOrder>::dequeueOrder(){
    // pass
    return -1;
}   