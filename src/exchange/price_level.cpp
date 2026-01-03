#include <sstream>
#include "exchange/book_order.hpp"
#include "exchange/price_level.hpp"

auto PriceLevel::enqueue(BookOrder* incoming) -> void {
    if (this->isEmpty()){
        head = tail = incoming;
    } else {
        tail->next = incoming;
        incoming->prev = tail;
        tail = incoming;
    }
    total_quantity += incoming->quantity;
}

auto PriceLevel::dequeue(BookOrder* to_remove) -> void {
    if (to_remove->prev){
        to_remove->prev->next = to_remove->next; // sets it to the chain after it
    } else {
        head = to_remove->next;
    }

    if (to_remove->next){
        to_remove->next->prev = to_remove->prev;
    } else {
        tail = to_remove->prev;
    }
    total_quantity -= to_remove->quantity;
}

auto PriceLevel::isEmpty() -> bool {
    return head == nullptr;
}

auto PriceLevel::clear() -> void {
    head = nullptr;
    tail = nullptr;
    total_quantity = 0;
}

auto PriceLevel::move(PriceLevel& move_from) -> void {
    head = move_from.head;
    tail = move_from.tail;
    total_quantity = move_from.total_quantity;
    move_from.clear();
}