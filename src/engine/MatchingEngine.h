#pragma once
#include <vector>
#include "core/OrderEvent.h"
using namespace std;

class MatchingEngine {
public:
    void process_batch(const vector<OrderEvent>& batch);
};
