# Evaluating the performance of lock-free data structures under realistic exchange workloads
Louis Tanak, University of Warwick Third Year Computer Science Project

## Abstract
Lock-free data structures allow multiple threads to operate safely on shared data without using locks. These have many real-word use cases in high-performance computing, low-latency systems and financial exchanges. Research has been conducted on their theoretical properties, however there is a gap in understanding how lock-free data structures perform under realistic workloads, such as trading exchange-like patterns. 

The work aims to implement various lock-free data structures and evaluate their performance against each other and traditional lock-based solutions. The data structures will be benchmarked against simulated exchange traffic to measure throughput, latency, scalability and completion ordering. Results will provide insights into the benefits, drawbacks and the suitable conditions for each data structure.
