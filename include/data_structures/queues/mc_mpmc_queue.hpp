#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/concurrentqueue.h"

/**
 * @class MCConcurrentQueue
 * @brief multi-producer, multi-consumer lock-free queue from moodycamel
 * @tparam TOrder Type of orders stored in the queue
 */
template <typename TOrder>
class MCConcurrentQueue : public IQueue<TOrder, MCConcurrentQueue<TOrder>> {

public:
    MCConcurrentQueue();
    auto enqueueOrder(TOrder &order) -> bool;
    auto dequeueOrder(TOrder &order) -> bool;
    auto getSize() -> uint64_t;
    auto isEmpty() -> bool;
    auto getFront(TOrder &order) -> bool;

private:
    moodycamel::ConcurrentQueue<TOrder> mcMPMCQueue_;
};
