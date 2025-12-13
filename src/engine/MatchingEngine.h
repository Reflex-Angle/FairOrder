#pragma once
#include <vector>
#include "core/OrderEvent.h"

class MatchingEngine {
public:
    void process_batch(const std::vector<OrderEvent>& batch);
};