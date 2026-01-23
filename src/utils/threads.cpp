#include "utils/threads.hpp"
#include <vector>
#include <atomic>
#include <thread>
#include <pthread.h>
#include <sched.h>

namespace lThread {
    using namespace std;

    void close(vector<thread> &threads){
        for (auto& thrd: threads) thrd.join();
    }

    void close(vector<thread> &producers, vector<thread> &consumers){
        for (auto& prod: producers) prod.join();
        for (auto& cons: consumers) cons.join();
    }

    void pin_thread(int cpu){
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu, &cpuset);
        int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }

}
