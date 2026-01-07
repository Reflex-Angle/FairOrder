cmake_minimum_required(VERSION 3.16)

project(MatchingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(engine
    src/main.cpp
    src/batching/MicroBatcher.cpp
    src/engine/MatchingEngine.cpp
    src/book/OrderBook.cpp
    src/simulation/Trader.cpp
    src/metrics/FairnessMetrics.cpp
    src/ui/CLI.cpp
)

target_include_directories(engine PRIVATE
    src
)