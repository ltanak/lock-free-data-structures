#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "hardware_logging/hardware_metrics.hpp"
#include "hardware_logging/thread_counter.hpp"
#include "utils/timing.hpp"

ThreadHardwareCounter::ThreadHardwareCounter() : initialised_(false) {
    fds_.fill(-1);
    initial_.fill(0);
    accumulated_.fill(0);
}

ThreadHardwareCounter::~ThreadHardwareCounter() {
    for (int fd : fds_){
        if (fd >= 0) close(fd);
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

    for (int i = 0; i < HW_COUNTERS; ++i){
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

        // tells kernel to start counting, but as we have disabled will only start when we enable
        ioctl(fds_[i], PERF_EVENT_IOC_ENABLE, 0);
    }
    initialised_ = true;
    return true;
}