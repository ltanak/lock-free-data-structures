#pragma once
#include <vector>
#include <sstream>

/**
 * TradesCycle struct
 * During each matching cycle, if trades are matched we output the list
   of all the trades matched with their prices and quantities, for analysis later
 */

struct TradesCycle {
    // one cycle is equivalent to one insertion
    int cycle; 

    // vectors of prices and their quantaties when matched
    // per-cycle can have multiple trades matched
    std::vector<double> prices;
    std::vector<uint32_t> quantities;
};
