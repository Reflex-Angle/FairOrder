#pragma once

#include <vector>
#include "core/OrderEvent.h"
#include "core/MatchingMode.h"
#include "book/OrderBook.h"
#include "metrics/FairnessMetrics.h"

class MatchingEngine {
public:
    explicit MatchingEngine(MatchingMode mode);
    
    void process_batch(const std::vector<OrderEvent>& batch, const std::vector<int>& trader_ids);
    void process_order(const OrderEvent& ev, int trader_id);
    
    MatchingMode get_mode() const { return mode_; }
    void set_mode(MatchingMode mode);
    
    const OrderBook& get_order_book() const { return order_book_; }
    const FairnessMetrics& get_metrics() const { return metrics_; }
    FairnessMetrics& get_metrics() { return metrics_; }

private:
    MatchingMode mode_;
    OrderBook order_book_;
    FairnessMetrics metrics_;
};