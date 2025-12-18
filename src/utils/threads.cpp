#include "utils/threads.hpp"
#include <vector>
#include <atomic>
#include <thread>

namespace lThread {
    using namespace std;

    void close(vector<thread> &threads){
        for (auto& thrd: threads) thrd.join();
    }

    void close(vector<thread> &producers, vector<thread> &consumers){
        for (auto& prod: producers) prod.join();
        for (auto& cons: consumers) cons.join();
    }

}
