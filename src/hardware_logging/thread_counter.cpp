#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "hardware_logging/hardware_metrics.hpp"
#include "hardware_logging/thread_counter.hpp"

#include "utils/timing.hpp"
#include "utils/counter.hpp"

ThreadHardwareCounter::ThreadHardwareCounter() : initialised_(false) {
    fds_.fill(-1);
    initial_.fill(0);
    accumulated_.fill(0);
}

ThreadHardwareCounter::~ThreadHardwareCounter() {
    if (!initialised_) return;
    
    for (int fd : fds_){
        if (fd >= 0) {
            ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
            close(fd);
        }
    }
}

/**
 * opens perf counter for each event
 * enables each counter, stores file descriptor
 */
bool ThreadHardwareCounter::setup() {
    struct perf_event_attr events[] = {
        {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CPU_CYCLES},
        {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_INSTRUCTIONS},
        {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CACHE_REFERENCES},
        {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CACHE_MISSES},
        {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS},
        {.type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_BRANCH_MISSES},
    };

    pid_t pid = 0; // measure only the calling thread
    int cpu = -1; // measure on whatever cpu the thread is running on
    unsigned long flags = 0;
    int group_fd = -1; // each counter is independent

    for (size_t i = 0; i < HW_COUNTERS; ++i){
        events[i].size = sizeof(struct perf_event_attr);
        events[i].disabled = 1; // start counter in disabled state, we enable it later
        events[i].exclude_kernel = 1; // exclude time spent in kernel mode
        events[i].exclude_hv = 1; // excludes hypervisor, makes sure only user-space time

        // opens the counter, syscall registers hardware event with kernel, file descriptor represents counter
        fds_[i] = syscall(SYS_perf_event_open, &events[i], pid, cpu, group_fd, flags);
        if (fds_[i] < 0){
            std::cerr << "Perf event failed: " << i << std::endl;
            return false;
        }
    }
    initialised_ = true;
    return true;
}

void ThreadHardwareCounter::start() {
    if (!initialised_) return;

    for (size_t i = 0; i < HW_COUNTERS; ++i){
        ioctl(fds_[i], PERF_EVENT_IOC_RESET, 0);
        ioctl(fds_[i], PERF_EVENT_IOC_ENABLE, 0);
    }
}

void ThreadHardwareCounter::stop() {
    if (!initialised_) return;

    for (size_t i = 0; i < HW_COUNTERS; ++i){
        ioctl(fds_[i], PERF_EVENT_IOC_DISABLE, 0);

        // default read format for perf fd is a single uint64_t counter value (instead of using rdpmc)
        uint64_t value = 0;
        ssize_t n = read(fds_[i], &value, sizeof(value));
        accumulated_[i] += (n == static_cast<ssize_t>(sizeof(value))) ? value : 0;
    }
}

void ThreadHardwareCounter::clear() {
    accumulated_.fill(0);
    initial_.fill(0);
}

HardwareMetrics ThreadHardwareCounter::snapshot() {
    HardwareMetrics metrics;
    metrics.cycles = static_cast<double>(accumulated_[0]);
    metrics.instructions = static_cast<double>(accumulated_[1]);
    metrics.cache_refs = static_cast<double>(accumulated_[2]);
    metrics.cache_misses = static_cast<double>(accumulated_[3]);
    metrics.branch_insts = static_cast<double>(accumulated_[4]);
    metrics.branch_misses = static_cast<double>(accumulated_[5]);
    return metrics;
}
