#pragma once

#include <vector>
#include "core/OrderEvent.h"

class MatchingEngine {
public:
    void process_batch(const std::vector<OrderEvent>& batch);

private:
    //helper to buy/sell independently
    void process_side(const std::vector<OrderEvent>& orders, Side side);
};