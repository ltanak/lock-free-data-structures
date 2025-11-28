#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/readerwriterqueue.h"

template <typename TOrder>
class MCLockFreeQueue : public IQueue<TOrder, MCLockFreeQueue<TOrder>> {

public:
    MCLockFreeQueue();
    bool enqueueOrder(TOrder &order);
    bool dequeueOrder(TOrder &order);
    uint64_t getSize();
    bool isEmpty();
    bool getFront(TOrder &order);

private:
    uint64_t size_;
    moodycamel::ReaderWriterQueue<TOrder> mcQueue_;
};