
#include <cstdint>
#include <utility>
#include "data_structures/ring_buffers/i_ring_buffer.hpp"
#include "data_structures/ring_buffers/ring.h"
#include "data_structures/ring_buffers/wilt_mpmc_nonblock_ring.hpp"
#include "order_simulation/benchmark_order.hpp"

#define NOT_IMPLEMENTED throw std::logic_error("Function not implemented.");

template<typename TOrder>
WiltMPMCNonBlockRing<TOrder>::WiltMPMCNonBlockRing(size_t capacity) 
    : wilt_ring_buffer_(capacity) {
}

template<typename TOrder>
bool WiltMPMCNonBlockRing<TOrder>::enqueueOrder(TOrder &order){
    wilt_ring_buffer_.try_write(order);
    return true;
}

template<typename TOrder>
bool WiltMPMCNonBlockRing<TOrder>::dequeueOrder(TOrder &order){
    wilt_ring_buffer_.try_read(order);
    return true;
}

template<typename TOrder>
bool WiltMPMCNonBlockRing<TOrder>::getFront(TOrder &order){
    NOT_IMPLEMENTED
    return false;
}

template<typename TOrder>
uint64_t WiltMPMCNonBlockRing<TOrder>::getSize(){
    size_t s = wilt_ring_buffer_.size();
    return static_cast<uint64_t>(s);
}

template<typename TOrder>
bool WiltMPMCNonBlockRing<TOrder>::isEmpty(){
    return wilt_ring_buffer_.size() == 0;
}

template class WiltMPMCNonBlockRing<BenchmarkOrder>;