#include "bench.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

struct UnpaddedCounters {
    std::atomic<std::uint64_t> counter1;
    std::atomic<std::uint64_t> counter2;
};

struct alignas(64) PaddedCounter {
    std::atomic<std::uint64_t> value;
};

struct PaddedCounters {
    PaddedCounter counter1;
    PaddedCounter counter2;
};

void run_false_sharing() {
    constexpr std::uint64_t increments = 100'000'000;

    std::cout << "False Sharing Benchmark\n";
    std::cout << "Increments per thread: " << increments << "\n";
    std::cout << "Threads: 2\n\n";

    {
        UnpaddedCounters counters;
        counters.counter1.store(0, std::memory_order_relaxed);
        counters.counter2.store(0, std::memory_order_relaxed);

        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&]() {
            for (std::uint64_t i = 0; i < increments; i++) {
                counters.counter1.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (std::uint64_t i = 0; i < increments; i++) {
                counters.counter2.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        std::cout << "Benchmark: Unpadded Counters\n";
        std::cout << "Total ns: " << total_ns << "\n";
        std::cout << "counter1: " << counters.counter1.load(std::memory_order_relaxed) << "\n";
        std::cout << "counter2: " << counters.counter2.load(std::memory_order_relaxed) << "\n\n";
    }

    {
        PaddedCounters counters;
        counters.counter1.value.store(0, std::memory_order_relaxed);
        counters.counter2.value.store(0, std::memory_order_relaxed);

        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&]() {
            for (std::uint64_t i = 0; i < increments; i++) {
                counters.counter1.value.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (std::uint64_t i = 0; i < increments; i++) {
                counters.counter2.value.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        std::cout << "Benchmark: Padded Counters\n";
        std::cout << "Total ns: " << total_ns << "\n";
        std::cout << "counter1: " << counters.counter1.value.load(std::memory_order_relaxed) << "\n";
        std::cout << "counter2: " << counters.counter2.value.load(std::memory_order_relaxed) << "\n\n";
    }
}