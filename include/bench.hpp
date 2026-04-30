#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>


template <typename F>
void run_benchmark(const std::string& name, int iterations, F&& func){
    // sink is used to make sure functions are optimized by compiler if the return value isn't used
    volatile std::uint64_t sink = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++){
        sink += func();
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    double avg_ns = static_cast<double>(total_ns) / iterations;

    std::cout << "Benchmark: " << name << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Total ns: " << total_ns << "\n";
    std::cout << "Average ns: " << avg_ns << "\n";
    std::cout << "Sink: " << sink << "\n\n";
}