#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/readerwriterqueue.h"

/**
 * @class MCLockFreeQueue
 * @brief single-consumer lock-free queue based on moodycamel's ReaderWriterQueue.
 *
 * @tparam TOrder Type of orders stored in the queue
 */
template <typename TOrder>
class MCLockFreeQueue : public IQueue<TOrder, MCLockFreeQueue<TOrder>> {

public:
    MCLockFreeQueue();
    auto enqueueOrder(TOrder &order) -> bool;
    auto dequeueOrder(TOrder &order) -> bool;
    auto getSize() -> uint64_t;
    auto isEmpty() -> bool;
    auto getFront(TOrder &order) -> bool;

private:
    moodycamel::ReaderWriterQueue<TOrder> mcQueue_;
};
