//#include "MicroBatcher.h"
#include "batching/MicroBatcher.h"
#include <iostream>
using namespace std;

MicroBatcher::MicroBatcher(TimeNs batch_window_ns)
    : window_ns(batch_window_ns),
      batch_start_ns(0),
      next_batch_id(1) {}

void MicroBatcher::submit(OrderEvent&& ev) {
    if (buffer.empty()) {
        batch_start_ns = ev.recv_time;
    }
    buffer.push_back(move(ev));
}

bool MicroBatcher::has_ready_batch() const {
    if (buffer.empty()) return false;
    return (buffer.back().recv_time - batch_start_ns) >= window_ns;
}

vector<OrderEvent> MicroBatcher::pop_batch() {
    for (auto& ev : buffer) {
        ev.batch_id = next_batch_id;
    }

    cout << "[MicroBatcher] Emitting batch "
         << next_batch_id
         << " with "
         << buffer.size()
         << " events\n";

    next_batch_id++;

    vector<OrderEvent> out;
    out.swap(buffer);
    return out;
}

/*
vector<OrderEvent> MicroBatcher::pop_batch() {
    for (auto& ev : buffer) {
        ev.batch_id = next_batch_id;
    }
    next_batch_id++;

    vector<OrderEvent> out;
    out.swap(buffer);
    return out;
}
*/