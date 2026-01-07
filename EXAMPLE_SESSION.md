# Example Session: Running FairOrder

A complete example session demonstrating how to use FairOrder and how to intepret the results.

## Step 1: Build the Project

```bash
cd build
cmake ..
cmake --build .
```

Expected output:
```
...
[ 12%] Building CXX object CMakeFiles/engine.dir/src/main.cpp.obj
[ 25%] Linking CXX executable engine.exe
...
[100%] Built target engine
```
This part may vary in between 0 and 100% and may show different stages for each build attempt. This is because CMake uses an incremental build system and is completely expected as long as it reaches 100%.

## Step 2: Run the Program

```bash
./engine
```

Expected output:
```
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║          FairOrder: Latency-Fair Matching Engine Demo            ║
║                                                                  ║
║     Demonstrating how micro-batching reduces latency arbitrage   ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝

Available Commands:
  help              - Show this help message
  menu              - Show main menu
  mode <naive|fair> - Set matching mode (naive = price-time, fair = batched)
  window <time>     - Set batch window (e.g., 100us, 1ms)
  simulate <N>      - Run simulation with N orders
  experiment        - Run comparative experiment (naive vs fair)
  metrics           - Show current fairness metrics
  book              - Show order book state
  reset             - Reset engine and metrics
  quit/exit         - Exit the program

Example usage:
  mode fair
  window 50us
  simulate 1000
  experiment

> 
```

## Step 3: Run a Comparative Experiment

This is the most explanatory demonstration. Type:

```
> experiment
```

### Expected Output (numbers may differ)
### Part 1: Naive Mode

```
╔══════════════════════════════════════════════════════════════════╗
║              COMPARATIVE EXPERIMENT: Naive vs Fair                ║
╚══════════════════════════════════════════════════════════════════╝

Running experiment with 1000 orders per mode...

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
MODE: NAIVE (Price-Time Priority)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

╔══════════════════════════════════════════════════════════════════╗
║ Running Simulation: 1000 orders                                  ║
║ Mode: Naive (Price-Time)                                         ║
╚══════════════════════════════════════════════════════════════════╝

Generating and processing orders...
Processed 1000 orders...

Simulation complete!

╔════════════════════════════════════════╗
║      FAIRNESS METRICS SUMMARY         ║
╠════════════════════════════════════════╣
║ Fairness Index:        0.18           ║
║ Latency Advantage      12%            ║
║ Reduction:                             ║
╚════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════╗
║                    DETAILED TRADER STATISTICS                   ║
╠══════════════════════════════════════════════════════════════════╣
║ Trader              │ Latency │ Orders │ Exec │ Win Rate │ Wins ║
╠══════════════════════════════════════════════════════════════════╣
║ FastTrader (5µs)    │ 5µs     │   245  │  198 │    92.3% │  182 ║
║ MediumTrader (25µs) │ 25µs    │   252  │  165 │    45.2% │   75 ║
║ SlowTrader (50µs)   │ 50µs    │   251  │  142 │    28.1% │   40 ║
║ VerySlowTrader      │ 100µs   │   252  │  125 │    18.5% │   23 ║
║ (100µs)             │         │        │      │         │      ║
╚══════════════════════════════════════════════════════════════════╝
```

**Key Observation**: In naive mode, the fast trader (5µs) wins 92% of trades, while slower traders get much lower win rates. This shows the latency advantage problem.

### Part 2: Fair Mode

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
MODE: FAIR (Latency-Fair Batched)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

╔══════════════════════════════════════════════════════════════════╗
║ Running Simulation: 1000 orders                                  ║
║ Mode: Fair (Batched)                                             ║
╚══════════════════════════════════════════════════════════════════╝

[MicroBatcher] Emitting batch 1 with 8 events
[MicroBatcher] Emitting batch 2 with 12 events
[MicroBatcher] Emitting batch 3 with 9 events
...
Generating and processing orders...
Processed 1000 orders...

Simulation complete!

╔════════════════════════════════════════╗
║      FAIRNESS METRICS SUMMARY         ║
╠════════════════════════════════════════╣
║ Fairness Index:        0.91           ║
║ Latency Advantage      89%            ║
║ Reduction:                             ║
╚════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════╗
║                    DETAILED TRADER STATISTICS                   ║
╠══════════════════════════════════════════════════════════════════╣
║ Trader              │ Latency │ Orders │ Exec │ Win Rate │ Wins ║
╠══════════════════════════════════════════════════════════════════╣
║ FastTrader (5µs)    │ 5µs     │   248  │  190 │    51.2% │   97 ║
║ MediumTrader (25µs) │ 25µs    │   251  │  178 │    48.9% │   87 ║
║ SlowTrader (50µs)   │ 50µs    │   250  │  172 │    47.8% │   82 ║
║ VerySlowTrader      │ 100µs   │   251  │  160 │    46.5% │   74 ║
║ (100µs)             │         │        │      │         │      ║
╚══════════════════════════════════════════════════════════════════╝
```

**Key Observation**: In fair mode, win rates are much more balanced (46-51%), showing that latency advantages are significantly reduced!

### Part 3: Comparison Table

```
╔══════════════════════════════════════════════════════════════════╗
║                    EXPERIMENT RESULTS                            ║
╠══════════════════════════════════════════════════════════════════╣
║ Metric                    │ Naive Mode │ Fair Mode │ Improvement║
╠══════════════════════════════════════════════════════════════════╣
║ Fairness Index            │      0.18  │     0.91  │    +405%   ║
║ Latency Advantage         │     12%    │    89%    │    +77%    ║
║ Reduction                 │            │           │            ║
╠══════════════════════════════════════════════════════════════════╣
║ Trader Win Rates:                                                ║
║ FastTrader (5µs)          │     92.3%  │    51.2%  │    -41%    ║
║ MediumTrader (25µs)        │     45.2%  │    48.9%  │    +4%     ║
║ SlowTrader (50µs)          │     28.1%  │    47.8%  │    +70%    ║
║ VerySlowTrader (100µs)     │     18.5%  │    46.5%  │    +151%   ║
╚══════════════════════════════════════════════════════════════════╝
```

## Step 4: Try Individual Commands

### Check Current Settings

```
> menu
```

Shows current mode and batch window settings.

### Run a Single Simulation

```
> mode fair
Mode set to: Fair (Batched)

> window 50us
Batch window set to: 50µs

> simulate 500
```

This runs 500 orders in fair mode with a 50 microsecond batch window.

### View Metrics Anytime

```
> metrics
```

Shows current fairness metrics and trader statistics.

### View Order Book

```
> book
```

Shows current best bid/ask and order book depth.

## What This Demonstrates

### The Problem (Naive Mode)
- **Fast traders dominate**: 90%+ win rates
- **Slow traders suffer**: <20% win rates
- **Unfair advantage**: Microsecond-level latency differences create huge disparities

### The Solution (Fair Mode)
- **Balanced win rates**: 46-51% for all traders
- **Fairness Index**: Jumps from 0.18 to 0.91
- **Latency advantage reduced**: From 12% to 89% reduction
- **Level playing field**: Orders within the batch window are treated equally

### Key Insights

1. **Fairness vs Performance**: Fair mode trades a small amount of latency (100µs batching) for massive fairness improvements

2. **Real-World Impact**: This demonstrates how exchanges could implement "speed bumps" to reduce latency arbitrage

3. **Measurable Results**: The metrics provide concrete evidence of fairness improvements

4. **Configurable**: You can adjust the batch window to find the right balance between fairness and responsiveness

## Tips for Best Results

1. **Run experiments multiple times**: Results vary slightly due to randomness
2. **Try different batch windows**: 
   - `window 10us` - More responsive, less fair
   - `window 100us` - Good balance (default)
   - `window 1ms` - Very fair, but slower
3. **Compare with different order counts**: Try 500, 1000, 5000 orders
4. **Watch the batch emissions**: In fair mode, you'll see batches being emitted

## Exit

```
> quit
Exiting...
```

---

This example session demonstrates the core value proposition: **latency-fair matching significantly reduces unfair advantages while maintaining market quality**.

