#pragma once
#include <string>
#include "Types.h"

enum class Side : uint8_t {
    BUY,
    SELL
};

enum class EventType : uint8_t {
    NEW,
    CANCEL
};

struct OrderEvent {
    EventType type;
    OrderID   order_id;
    std::string instrument;
    Side      side;
    Price     price;
    Qty       qty;

    TimeNs    recv_time;   // when engine received it
    BatchID   batch_id;    // assigned by MicroBatcher
};