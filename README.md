# Evaluating the performance of lock-free data structures under realistic exchange workloads
Louis Tanak, University of Warwick Third Year Computer Science Project

## Abstract
Lock-free data structures allow multiple threads to operate safely on shared data without using locks. These have many real-word use cases in high-performance computing, low-latency systems and financial exchanges. Research has been conducted on their theoretical properties, however there is a gap in understanding how lock-free data structures perform under realistic workloads, such as trading exchange-like patterns. 

The work aims to implement various lock-free data structures and evaluate their performance against each other and traditional lock-based solutions. The data structures will be benchmarked against simulated exchange traffic to measure throughput, latency, scalability and completion ordering. Results will provide insights into the benefits, drawbacks and the suitable conditions for each data structure.

## Running the Code
Firstly, compile the code:
`./compile.sh`

Run the specific test you would like:
`./run.sh {test_type} {thread_count} {order_limit}`

Test types are: `stress` and `order`.

Thread count represents how many threads you would like to use to run the test. Note: if you try to use mutliple threads for non MPMC-style lock-free data structures, you will experience unexpected behaviour.

Order limit represents how many orders will be put through the system, equally distributed amongst the threads.

## Benchmarking
Once you have ran the code, a .csv will be created in `src/benchmarking/csvs` in the corresponding folder. They have their associated timestamp in the file name.

With this .csv, you can do what you would like with it, however my own Python code has been implemented to analyse my own results.

### Running Python Code
Uses pixi as virtual environment, script is also already setup. First cd/ into:
`cd src/benchmarking/python`
Then run:
`pixi run run`
Note: If Python files do not work that is because they are actively modified to test and graph different things. Future improvements will create fully generated reports containing all the information required.
