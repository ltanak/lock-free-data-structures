#pragma once
#include <vector>
#include <sstream>

/**
 * TradesCycle struct
 * During each matching cycle, if trades are matched we output the list
   of all the trades matched with their prices and quantities, for analysis later
 */

struct TradesCycle {
    int cycle;
    std::vector<double> prices;
    std::vector<uint32_t> quantities;
};