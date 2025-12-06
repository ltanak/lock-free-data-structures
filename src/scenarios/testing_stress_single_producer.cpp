/**
 * @brief Single producer stress test
 * Purpose of test is to benchmark the throughput of a single producer adding to a data structure
 * A single thread will be generating orders and pushing them to the data structure
 * 
 * Logic for this code has been moved into include/scenarios in corresponding .hpp files
 * This allows for it to be templated correctly, so the data structure only needs to be defined once in main.cpp when benchmarking
 */