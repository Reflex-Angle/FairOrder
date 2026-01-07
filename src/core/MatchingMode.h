#pragma once

enum class MatchingMode {
    NAIVE_PRICE_TIME,      // Traditional: process immediately, recv_time breaks ties
    LATENCY_FAIR_BATCHED   // Fair: batch orders, ignore recv_time within batch
};

