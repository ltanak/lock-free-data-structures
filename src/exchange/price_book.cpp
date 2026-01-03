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
    uint32_t new_base_ticks = new_centre - (NUM_LEVELS / 2);
    shift(new_base_ticks);
}

auto PriceBook::shift(uint32_t new_ticks) -> void {
    PriceLevel new_levels[NUM_LEVELS];
    uint64_t new_bitmap[BITMAP_WORDS] = {0};

    for (int i = 0; i < NUM_LEVELS; ++i){
        new_levels[i].clear();
    }

    for (int i = 0; i < NUM_LEVELS; ++i){
        if (levels[i].isEmpty()) continue;

        uint32_t abs_price = base_price_ticks + i;
        uint32_t new_index = abs_price - new_ticks;

        if (new_index >= NUM_LEVELS) continue;

        new_levels[new_index].move(levels[i]);

        // shows that this price_level is active, then activates the specific bit
        new_bitmap[new_index / 64] |= (1ULL << (new_index % 64));
    }
    // then commits the results into the main bitmap and levels
    for (int i = 0; i < NUM_LEVELS; ++i){
        levels[i].move(new_levels[i]);
    }
    std::memcpy(bitmap, new_bitmap, sizeof(bitmap));
    base_price_ticks = new_ticks;
}

auto PriceBook::priceToIndex(uint32_t price) -> int {
    return static_cast<int>(price) - static_cast<int>(base_price_ticks);
}

auto PriceBook::addOrder(BookOrder* order) -> void {
    int index = priceToIndex(order->price_ticks);
    if (index < 0 || index >= NUM_LEVELS) return;
    levels[index].enqueue(order);
    setBit(index);
}

auto PriceBook::removeOrder(BookOrder* order) -> void {
    int index = priceToIndex(order->price_ticks);
    levels[index].dequeue(order);
    if (levels[index].isEmpty()) clearBit(index);
}

auto PriceBook::setBit(int index) -> void {
    bitmap[index / 64] |= (1ULL << (index % 64));
}

auto PriceBook::clearBit(int index) -> void {
    bitmap[index / 64] &= ~(1ULL << (index % 64));
}

auto PriceBook::bestPriceLevel() const -> int {
    for (int word = 0; word < BITMAP_WORDS; ++word){
        if (bitmap[word]){
            return (word * 64) + __builtin_ctzll(bitmap[word]); // counts trailing zeros (for price calculation)
        }
    }
    return -1;
}