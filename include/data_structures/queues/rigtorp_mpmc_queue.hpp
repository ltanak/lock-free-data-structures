#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/MPMCQueue.h"

/**
 * @class Rigtorp MPMC queue
 * @brief multi-producer, multi-consumer lock-free queue from rigtorp.
 *
 * @tparam TOrder Type of orders stored in the queue
 */
template <typename TOrder>
class RigtorpMPMCQueue : public IQueue<TOrder, RigtorpMPMCQueue<TOrder>> {

public:
    RigtorpMPMCQueue(size_t capacity = 100000);
    auto enqueueOrder(TOrder &order) -> bool;
    auto dequeueOrder(TOrder &order) -> bool;
    auto getSize() -> uint64_t;
    auto isEmpty() -> bool;
    auto getFront(TOrder &order) -> bool;

private:
    rigtorp::MPMCQueue<TOrder> rigtorp_queue_;
};
