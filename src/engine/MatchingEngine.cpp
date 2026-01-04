#include "engine/MatchingEngine.h"
#include <algorithm>
#include <iostream>

void MatchingEngine::process_batch(const std::vector<OrderEvent>& batch) {
    if (batch.empty()) return;

    std::cout << "\n=== Processing Batch "
              << batch.front().batch_id
              << " ===\n";

    std::vector<OrderEvent> buys;
    std::vector<OrderEvent> sells;

    for (const auto& ev : batch) {
        if (ev.side == Side::BUY)
            buys.push_back(ev);
        else
            sells.push_back(ev);
    }

    process_side(buys, Side::BUY);
    process_side(sells, Side::SELL);
}

void MatchingEngine::process_side(const std::vector<OrderEvent>& orders, Side side) {
    if (orders.empty()) return;

    std::vector<OrderEvent> sorted = orders;

    if (side == Side::BUY) {
        std::sort(sorted.begin(), sorted.end(),
            [](const OrderEvent& a, const OrderEvent& b) {
                if (a.price != b.price)
                    return a.price > b.price;
                return a.order_id < b.order_id;
            });
        std::cout << "BUY priority:\n";
    } else {
        std::sort(sorted.begin(), sorted.end(),
            [](const OrderEvent& a, const OrderEvent& b) {
                if (a.price != b.price)
                    return a.price < b.price;
                return a.order_id < b.order_id;
            });
        std::cout << "SELL priority:\n";
    }

    for (const auto& o : sorted) {
        std::cout
            << "  OrderID=" << o.order_id
            << " Price=" << o.price
            << " Qty=" << o.qty
            << "\n";
    }
}
