#pragma once
#include <vector>
#include "core/OrderEvent.h"

class MicroBatcher {
public:
    explicit MicroBatcher(TimeNs batch_window_ns);

    void submit(OrderEvent&& ev);
    bool has_ready_batch() const;
    std::vector<OrderEvent> pop_batch();

private:
    TimeNs window_ns;
    TimeNs batch_start_ns;
    BatchID next_batch_id;

    std::vector<OrderEvent> buffer;
};
