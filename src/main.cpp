#include <iostream>
#include <chrono>
#include <thread>

#include "batching/MicroBatcher.h"
#include "engine/MatchingEngine.h"

using namespace std;

static TimeNs now_ns() {
    return chrono::duration_cast<chrono::nanoseconds>(
        chrono::steady_clock::now().time_since_epoch()
    ).count();
}

int main() {

    /*
    cout << "Engine started.\n";
    MicroBatcher batcher(100'000); // 100 µs
    MatchingEngine engine;

    // Simulate incoming orders
    for (int i = 0; i < 10; i++) {
        OrderEvent ev;
        ev.type = EventType::NEW;
        ev.order_id = i + 1;
        ev.instrument = "ABC";
        ev.side = Side::BUY;
        ev.price = 100;
        ev.qty = 10;
        ev.recv_time = now_ns();

        batcher.submit(move(ev));

        this_thread::sleep_for(chrono::microseconds(30));
    }


    while (batcher.has_ready_batch()) {
        auto batch = batcher.pop_batch();
        engine.process_batch(batch);
        cout << "Processed batch of size " << batch.size() << "\n";
    }
    */

    std::cout << "=== Fair Priority Test ===\n";

    MicroBatcher batcher(100'000); // 100 µs
    MatchingEngine engine;

    // Same price, different IDs
    {
        OrderEvent a{EventType::NEW, 5, "ABC", Side::BUY, 100, 10, now_ns()};
        OrderEvent b{EventType::NEW, 2, "ABC", Side::BUY, 100, 10, now_ns()};
        OrderEvent c{EventType::NEW, 7, "ABC", Side::BUY, 101, 10, now_ns()};

        batcher.submit(std::move(a));
        batcher.submit(std::move(b));
        batcher.submit(std::move(c));

    }

    // Force flush
    auto batch = batcher.pop_batch();
    engine.process_batch(batch);


}
