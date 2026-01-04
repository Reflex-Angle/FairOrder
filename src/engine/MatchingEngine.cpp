//#include "MatchingEngine.h"
#include "engine/MatchingEngine.h"
#include <iostream>
using namespace std;

void MatchingEngine::process_batch(const vector<OrderEvent>& batch) {
    cout << "[MatchingEngine] Processing batch "
         << batch.front().batch_id
         << "\n";

    for (const auto& ev : batch) {
        cout
            << "  OrderID=" << ev.order_id
            << " Side=" << (ev.side == Side::BUY ? "BUY" : "SELL")
            << " Price=" << ev.price
            << " Qty=" << ev.qty
            << " BatchID=" << ev.batch_id
            << "\n";
    }
}
