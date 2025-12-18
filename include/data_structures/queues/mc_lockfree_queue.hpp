#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/readerwriterqueue.h"

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
    uint64_t size_;
    moodycamel::ReaderWriterQueue<TOrder> mcQueue_;
};