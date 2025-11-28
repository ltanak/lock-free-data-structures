#pragma once
#include <type_traits>

template <typename DS>
decltype(auto) dequeueWrapper(DS& ds) {

    using Ret = decltype(ds.dequeue());

    if constexpr (std::is_void_v<Ret>) { // if return type is void
        ds.dequeue();
        return;
    } else {
        return ds.dequeue();   // return the order
    }
}