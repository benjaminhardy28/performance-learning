# False Sharing Benchmark Summary

## What This Benchmark Tested

This benchmark tested the performance impact of false sharing between two threads.

False sharing happens when multiple threads modify different variables that happen to sit on the same CPU cache line. Even though the threads are not logically sharing the same variable, the CPU cache coherency system tracks ownership at the cache-line level. This can cause the cache line to bounce between cores and slow the program down.

The benchmark compared two layouts:

1. Unpadded counters
2. Cache-line padded counters

Each test used two threads. Each thread incremented its own counter 100,000,000 times.

## Test Setup

### Unpadded Counters

The unpadded version stores two atomic counters directly next to each other:

struct UnpaddedCounters {
    std::atomic<std::uint64_t> counter1;
    std::atomic<std::uint64_t> counter2;
};

In memory, the counters are likely close together:

[counter1][counter2][unused space...]

Since each uint64_t counter is 8 bytes and a typical cache line is 64 bytes, both counters can fit on the same cache line.

The two threads do this:

Thread 1 increments counter1
Thread 2 increments counter2

Even though the threads modify different counters, both counters may live on the same cache line. This can cause false sharing.

### Padded Counters

The padded version places each counter in a cache-line-aligned wrapper:

struct alignas(64) PaddedCounter {
    std::atomic<std::uint64_t> value;
};

struct PaddedCounters {
    PaddedCounter counter1;
    PaddedCounter counter2;
};

The intended memory layout is:

[counter1.value + padding...]
[counter2.value + padding...]

This separates the counters so each one is on its own cache line. That reduces cache coherency traffic between the two threads.

## How It Works

Each benchmark creates two worker threads:

- Thread 1 increments the first counter 100,000,000 times.
- Thread 2 increments the second counter 100,000,000 times.

Both counters use relaxed atomic increments:

fetch_add(1, std::memory_order_relaxed)

Relaxed memory ordering was used because the benchmark is not testing synchronization ordering. It is testing the cost of atomic modification and cache-line contention.

The final counter values are printed to make sure the work was actually performed.

Expected final values:

counter1: 100,000,000
counter2: 100,000,000

## Thread Scheduling Note

This benchmark creates two threads, but it does not explicitly pin them to specific CPU cores on macOS. The operating system scheduler controls where the threads run.

The large slowdown in the unpadded version is consistent with the threads running on separate cores for much of the benchmark and experiencing false sharing. However, exact core placement is not guaranteed.

## Results

Each run used:

- Increments per thread: 100,000,000
- Threads: 2

| Run | Unpadded Counters | Padded Counters | Speedup |
|---:|---:|---:|---:|
| 1 | 960,742,333 ns | 226,218,083 ns | 4.25x |
| 2 | 986,773,750 ns | 227,198,000 ns | 4.34x |
| 3 | 974,627,958 ns | 225,833,833 ns | 4.32x |
| 4 | 980,984,708 ns | 225,526,875 ns | 4.35x |
| 5 | 997,575,125 ns | 225,683,500 ns | 4.42x |
| 6 | 994,541,750 ns | 225,237,000 ns | 4.42x |
| 7 | 965,450,542 ns | 225,766,208 ns | 4.28x |
| 8 | 975,249,917 ns | 225,627,292 ns | 4.32x |
| 9 | 965,556,291 ns | 226,560,583 ns | 4.26x |
| 10 | 996,129,583 ns | 225,747,917 ns | 4.41x |

## Average Results

| Layout | Average Total Time |
|---|---:|
| Unpadded Counters | 979,763,196 ns |
| Padded Counters | 225,939,929 ns |

Average speedup:

979,763,196 / 225,939,929 = 4.34x

The padded version was approximately 4.34x faster than the unpadded version.

Another way to describe this is that padding reduced total runtime by about 76.9% compared to the unpadded version.

## Interpretation

The unpadded version was much slower because both atomic counters were likely located on the same cache line. When two cores repeatedly modify different values on the same cache line, the cache line must repeatedly transfer ownership between cores.

This creates unnecessary cache coherency traffic.

The padded version avoids this by separating the two counters onto different cache lines. Each thread can modify its own counter with much less interference from the other thread.

## Takeaway

This benchmark demonstrates that data layout matters in multithreaded code.

Two variables can be independent at the source-code level but still interfere with each other at the hardware level if they share the same cache line.

The key lesson is:

Avoid placing frequently modified thread-local or per-core data on the same cache line.
False sharing is a performance bug, not a correctness bug. The program still produces the right counter values, but it runs much slower because of unnecessary cache-coherency traffic.