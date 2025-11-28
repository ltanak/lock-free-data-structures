#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/concurrentqueue.h"

template <typename TOrder>
class MCConcurrentQueue : public IQueue<TOrder, MCConcurrentQueue<TOrder>> {

public:
    MCConcurrentQueue();
    bool enqueueOrder(TOrder &order);
    void dequeueOrder();
    TOrder dequeueOrderV();
    uint64_t getSize();
    bool isEmpty();
    TOrder getFront();

private:
    uint64_t size_;
    moodycamel::ConcurrentQueue<TOrder> mcMPMCQueue_;
};