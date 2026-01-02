#include <sstream>
#include <bitset>
#include <cstring>
#include <algorithm>
#include <iterator>
#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"

auto PriceBook::initialise(uint32_t centre) -> void {
    centre_price_ticks = centre;
    base_price_ticks = centre - (NUM_LEVELS / 2);
    std::fill(std::begin(levels), std::end(levels), PriceLevel{});
    std::memset(bitmap, 0, sizeof(bitmap));
}

auto PriceBook::recentre(uint32_t new_centre) -> void {
    centre_price_ticks = new_centre;
    base_price_ticks = new_centre - (NUM_LEVELS / 2);


}

auto PriceBook::shift(uint32_t new_ticks) -> void {

    return;
}

inline auto PriceBook::priceToIndex(uint32_t price) -> int {
    return -1;
}