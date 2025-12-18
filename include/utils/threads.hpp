#pragma once

#include <thread>
#include <atomic>
#include <vector>

namespace lThread {
    using namespace std;

    void close(vector<thread> &threads);
    void close(vector<thread> &producers, vector<thread> &consumers);

}
