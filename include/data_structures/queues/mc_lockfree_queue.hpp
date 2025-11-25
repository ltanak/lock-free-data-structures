#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/readerwriterqueue.h"

template <typename TOrder>
class MCLockFreeQueue : public IQueue<TOrder, MCLockFreeQueue<TOrder>> {

public:
    MCLockFreeQueue();
    bool enqueueOrder(TOrder &order);
    void dequeueOrder();
    uint64_t getSize();
    bool isEmpty();
    TOrder getFront();

private:
    uint64_t size_;
    moodycamel::ReaderWriterQueue<TOrder> mcQueue_;
};