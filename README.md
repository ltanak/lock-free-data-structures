# Evaluating the performance of lock-free data structures under realistic exchange workloads
Louis Tanak, University of Warwick Third Year Computer Science Project

## Abstract
Lock-free data structures allow multiple threads to operate safely on shared data without using locks. These have many real-word use cases in high-performance computing, low-latency systems and financial exchanges. Research has been conducted on their theoretical properties, however there is a gap in understanding how lock-free data structures perform under realistic workloads, such as trading exchange-like patterns. 

The work aims to implement various lock-free data structures and evaluate their performance against each other and traditional lock-based solutions. The data structures will be benchmarked against simulated exchange traffic to measure throughput, latency, scalability and completion ordering. Results will provide insights into the benefits, drawbacks and the suitable conditions for each data structure.

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
Once you have ran the code, a .csv will be created in `src/benchmarking/csvs` in the corresponding folder. They have their associated timestamp in the file name.

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

Note: The plotting code is actively evolving. The report mode is the most stable path for benchmarking.
