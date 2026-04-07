# Evaluating the performance of lock-free data structures under realistic exchange workloads
Louis Tanak, University of Warwick Third Year Computer Science Project 2026.

## Abstract
Lock-free data structures are concurrent data structures that allow multiple threads to operate without the use of locks for synchronisation. They utilise atomic hardware instructions such as compare-and-swap (\acrshort{CAS}) or fetch-and-add (\acrshort{FAA}) to guarantee that at least one thread makes progress in a finite number of steps. Due to their ability to reduce contention and improve scalability, lock-free data structures have various applications primarily in high-performance systems such as web servers, physics engines and high-frequency trading systems. 

Modern research covers different implementations of various lock-free data structures, to improve their latency and scalability with increasing thread counts. However, there currently lacks standardised frameworks to empirically evaluate lock-free data structures. Further to this, studies primarily focus on throughput and latency under synthetic workloads, whilst little work investigates order-preservation properties and their effects on downstream systems. These are critical properties in applications such as financial exchanges, where fairness and correctness depend on ordering guarantees.

This project evaluates the performance of lock-free data structures under realistic exchange workloads. We present a low-latency benchmarking framework, which provides concurrent measurement of latencies, order-preservation and implements a price-time priority matching engine to assess the downstream effects of ordering anomalies. Using the framework, we present a performance analysis of various lock-free data structure implementations and their characteristics. We conclude that the framework can effectively evaluate performance and ordering properties, providing a reusable suite for evaluating lock-free data structures.

## Supervisor Running Instructions
This section is a quickstart intended for the project supervisor. It walks through running any one of the six lock-free data-structure implementations out of the box and viewing the generated report.

### 1. Compile the project
From the repository root, run:
```bash
./compile.sh
```

### 2. Select the data structure to evaluate
Open `src/main.cpp`. Near the top of `main()` you will find six clearly-labelled blocks under the **DATA STRUCTURE SELECTION** banner:

1. `RegularQueue` - lock-based baseline
2. `MCConcurrentQueue` - moodycamel ConcurrentQueue (MPMC)
3. `MCLockFreeQueue` - moodycamel ReaderWriterQueue (SPSC only)
4. `WiltMPMCBlockRing` - Wilt MPMC blocking ring buffer
5. `WiltMPMCNonBlockRing` - Wilt MPMC non-blocking ring buffer
6. `RigtorpMPMCQueue` - Rigtorp MPMC queue *(active by default)*

Uncomment **exactly one** block and comment out the others, then re-run `./compile.sh`.

By default, `RigtorpMPMCQueue` is uncommented.

### 3. Run the benchmark
The recommended testing scenario runs both the stress and order scenarios under the same run ID:
```bash
./run.sh --all 4 1000
```
The arguments are:
- `4` - number of producer/consumer threads (use `1` for SPSC-only structures such as the moodycamel ReaderWriterQueue)
- `1000` - total orders driven through the system

The script prints a run ID of the form `XXXXXXXXXX` at the start and end of the run. **Copy this run ID**, as it is needed for the report step.

### 4. Generate the HTML report
Switch into the Python benchmarking directory and generate the report for the run ID from step 3:
```bash
cd src/benchmarking/python
pixi run report --run-id=XXXXXXXXXX
```
(Replace `XXXXXXXXXX` with the actual ID printed by `run.sh`.)

The first invocation of `pixi run` will download the Python environment, with subsequent calls being faster once the environment is resolved.

### 5. View the report
The generated HTML report is written to:
```
results/reports/report_<run_id>.html
```
Open this file in any browser. It contains latency plots, ordering summaries, exchange-matching results and hardware counter data for the selected data structure.

### Full supervisor walkthrough (copy-paste)
```bash
./compile.sh
# edit src/main.cpp, uncomment the desired data structure, re-run compile.sh
./run.sh --all 4 1000
# note the run ID printed by run.sh
cd src/benchmarking/python
pixi run report --run-id=<RUN_ID>
# open results/reports/report_<RUN_ID>.html in a browser
```

## Adding your own Lock-free Data Structure
To begin benchmarking your lock-free data structure, perform the following:
1. Add your relevant lock-free data structures includes in `include/data_structures/{queues | ring_buffers}`
2. Within the same directory, create a file `{YOUR_CHOSEN_NAME}.hpp` with this structure:
```C++
#pragma once

#include "data_structures/ring_buffers/i_ring_buffer.hpp"
#include "data_structures/queues/i_queue.hpp"

// Ensure you have included your lock-free data structure code header files!

template <typename TOrder>
class {CLASS_NAME} : public {IRing | IQueue}<TOrder, {CLASS_NAME}<TOrder>> {

public:
    {CLASS_NAME}();
    auto enqueueOrder(TOrder &order) -> bool;
    auto dequeueOrder(TOrder &order) -> bool;
    auto getSize() -> uint64_t;
    auto isEmpty() -> bool;
    auto getFront(TOrder &order) -> bool;
private:
    // Add your lock free data structure here, e.g:
    // wilt::Ring<TOrder> wilt_ring_buffer_;
};
```
3. Now in `src/data_structures/{queues | ring_buffers}` create your .cpp file, with this template:
```C++
#include <cstdint>
#include <utility>
#include "order_simulation/benchmark_order.hpp"

/*
    DON'T FORGET TO INCLUDE YOUR HEADER FILE!
*/

#include "data_structures/{queues | ring_buffers}/{YOUR_FILE_NAME}.hpp"

#define NOT_IMPLEMENTED throw std::logic_error("Function not implemented.");


// NOTE: Your instantiation can look however you want, just ensure you reflect the changes properly!
template<typename TOrder>
{YOUR_CLASS_NAME}<TOrder>::{YOUR_CLASS_NAME} {

}

template<typename TOrder>
bool {YOUR_CLASS_NAME}<TOrder>::enqueueOrder(TOrder &order){
    // IMPLEMENT YOUR CODE HERE
}

template<typename TOrder>
bool {YOUR_CLASS_NAME}<TOrder>::dequeueOrder(TOrder &order){
    // IMPLEMENT YOUR CODE HERE
}

template<typename TOrder>
bool {YOUR_CLASS_NAME}<TOrder>::getFront(TOrder &order){
    // IMPLEMENT YOUR CODE HERE
}

template<typename TOrder>
uint64_t {YOUR_CLASS_NAME}<TOrder>::getSize(){
    // IMPLEMENT YOUR CODE HERE
}

template<typename TOrder>
bool {YOUR_CLASS_NAME}<TOrder>::isEmpty(){
    // IMPLEMENT YOUR CODE HERE
}

template class {YOUR_CLASS_NAME}<BenchmarkOrder>;
```

4. At the bottom of `src/benchmarking/cpp/benchmarking.cpp add:
```C++
template class BenchmarkWrapper<{YOUR_CLASS_NAME}<BenchmarkOrder>, BenchmarkOrder>;
```
5. In `src/main.cpp` add the follwing in front of the switch statement:
```C++
{YOUR_CLASS_NAME}<BenchmarkOrder> lock_free_datastructure;
BenchmarkWrapper<{YOUR_CLASS_NAME}<BenchmarkOrder>, BenchmarkOrder> wrapper(lock_free_datastructure, params);
```

You are now ready to benchmark your lock-free data structure!

## Running the Code
Firstly, compile the code:
`./compile.sh`

Run the specific test you would like:
`./run.sh {test_type} {thread_count} {order_limit}`

Test types are: `stress` and `order`.

Thread count represents how many threads you would like to use to run the test. Note: if you try to use mutliple threads for non MPMC-style lock-free data structures, you will experience unexpected behaviour.

Order limit represents how many orders will be put through the system, equally distributed amongst the threads.

### Useful flags
The `run.sh` script supports additional benchmarking flags:
- `--all` runs both `stress` and `order` with the same run ID
- `--seed=UINT64` fixes the random seed for repeatable results
- `--perf` records a perf profile
- `--valgrind` runs memcheck
- `--cachegrind` runs cachegrind
- `--gdb` runs under gdb (single test only)

To see the full list of flags at any time:
`./run.sh --help`

Examples:
- `./run.sh --all 4 100000`
- `./run.sh --seed=42 order 4 100000`
- `./run.sh --perf stress 8 1000000`
- `./run.sh --valgrind order 1 1000`

## Benchmarking
Once you have run the code, CSV files will be created in the `results` directory with the following structure:
- `results/stress_testing/` - Contains latencies and hardware CSVs/graphs from stress testing
  - `latencies/` - Latency measurements
  - `hardware/` - Hardware counter data (from stress tests)
- `results/order_testing/` - Contains ordering, exchange, and hardware CSVs from order testing
  - `ordering/` - Order preservation testing
  - `exchange/` - Matching engine results
  - `hardware/` - Hardware counter data (from order tests)
- `results/reports/` - Generated HTML reports

Note: Hardware metrics are collected for both test types and stored in the corresponding directory.
All files have their associated timestamp in the file name.

### Report generation (recommended)
The out‑of‑the‑box way to benchmark and compare results is the HTML report. It aggregates latency, ordering, and exchange sections with graphs and summaries:
- `cd src/benchmarking/python`
- `pixi run report`

You can also target a specific run ID:
- `pixi run report --run-id <RUN_ID>`

Report flags are available too. To list all plotting/report flags:
- `pixi run main --help`

### Manual plotting (optional)
If you want direct plots instead of the report:
- `pixi run main`
- Or: `pixi run -- python -m scripts.main --latencies --ordering --exchange`

If you would prefer to look at the code, please see: `pixi.toml` and `main.py` (both are unique names in this codebase!)

### Cross-run latency comparison
To overlay latency graphs from multiple runs onto a single image (useful for comparing different data structures), see `src/benchmarking/python/image_editing/compare_runs.py` or run:
- `pixi run compare --run-ids 1234567 7654321 1122334`

Output images are saved into `src/benchmarking/python/image_editing/`.

Note: The plotting code is actively evolving. The report mode is the most stable path for benchmarking.