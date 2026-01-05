#pragma once
#include <vector>
#include <sstream>

struct TradesCycle {
    int cycle;
    std::vector<double> prices;
    std::vector<uint32_t> quantities;
};