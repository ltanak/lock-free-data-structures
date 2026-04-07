
#include <cstdint>
#include <utility>
#include "data_structures/ring_buffers/i_ring_buffer.hpp"
#include "data_structures/ring_buffers/ring.h"
#include "data_structures/ring_buffers/wilt_mpmc_blocking_ring.hpp"
#include "order_simulation/benchmark_order.hpp"

#define NOT_IMPLEMENTED throw std::logic_error("Function not implemented.");

template<typename TOrder>
WiltMPMCBlockRing<TOrder>::WiltMPMCBlockRing(size_t capacity) 
    : wilt_ring_buffer_(capacity) {
}

template<typename TOrder>
bool WiltMPMCBlockRing<TOrder>::enqueueOrder(TOrder &order){
    wilt_ring_buffer_.write(order);
    return true;
}

template<typename TOrder>
bool WiltMPMCBlockRing<TOrder>::dequeueOrder(TOrder &order){
    wilt_ring_buffer_.read(order);
    return true;
}

template<typename TOrder>
bool WiltMPMCBlockRing<TOrder>::getFront(TOrder &order){
    NOT_IMPLEMENTED
    return false;
}

template<typename TOrder>
uint64_t WiltMPMCBlockRing<TOrder>::getSize(){
    size_t s = wilt_ring_buffer_.size();
    return static_cast<uint64_t>(s);
}

template<typename TOrder>
bool WiltMPMCBlockRing<TOrder>::isEmpty(){
    return wilt_ring_buffer_.size() == 0;
}

template class WiltMPMCBlockRing<BenchmarkOrder>;
