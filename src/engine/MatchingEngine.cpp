//#include "MatchingEngine.h"
#include "engine/MatchingEngine.h"
#include <iostream>

void MatchingEngine::process_batch(const std::vector<OrderEvent>& batch) {
    std::cout << "[MatchingEngine] Processing batch "
              << batch.front().batch_id
              << "\n";

    for (const auto& ev : batch) {
        std::cout
            << "  OrderID=" << ev.order_id
            << " Side=" << (ev.side == Side::BUY ? "BUY" : "SELL")
            << " Price=" << ev.price
            << " Qty=" << ev.qty
            << " BatchID=" << ev.batch_id
            << "\n";
    }
}
