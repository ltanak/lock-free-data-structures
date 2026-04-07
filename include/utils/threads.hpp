#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <pthread.h>
#include <sched.h>

/**
 * louis-thread functions, primarily for thread collection and CPU-pinning
 */
namespace lThread {
    using namespace std;

    void close(vector<thread> &threads);
    void close(vector<thread> &producers, vector<thread> &consumers);
    void pin_thread(int cpu);
}
