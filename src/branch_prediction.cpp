#include "bench.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

#if defined(__clang__) || defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

NOINLINE std::uint64_t true_path(int value) {
    return static_cast<std::uint64_t>(value * 3 + 1);
}

NOINLINE std::uint64_t false_path(int value) {
    return static_cast<std::uint64_t>(value * 7 + 2);
}

void run_branch_prediction() {
    constexpr int n = 10'000'000;
    constexpr int iterations = 30;
    constexpr int threshold = 128;

    std::vector<int> predictable(n);
    std::vector<int> unpredictable(n);

    for (int i = 0; i < n; i++) {
        predictable[i] = i % 256;
        unpredictable[i] = i % 256;
    }

    std::sort(predictable.begin(), predictable.end());

    std::mt19937 rng(42);
    std::shuffle(unpredictable.begin(), unpredictable.end(), rng);

    std::cout << "Branch Prediction and Branchless Computation Benchmark\n";
    std::cout << "Elements: " << n << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Threshold: " << threshold << "\n\n";

    std::cout << "Test 1: Branch Prediction Isolation\n";
    std::cout << "Uses noinline functions to make the branch harder to optimize away.\n\n";

    run_benchmark("Predictable Branch with noinline calls", iterations, [&]() -> std::uint64_t {
        std::uint64_t sum = 0;

        for (int i = 0; i < n; i++) {
            int value = predictable[i];

            if (value < threshold) {
                sum += true_path(value);
            } else {
                sum += false_path(value);
            }
        }

        return sum;
    });

    run_benchmark("Unpredictable Branch with noinline calls", iterations, [&]() -> std::uint64_t {
        std::uint64_t sum = 0;

        for (int i = 0; i < n; i++) {
            int value = unpredictable[i];

            if (value < threshold) {
                sum += true_path(value);
            } else {
                sum += false_path(value);
            }
        }

        return sum;
    });

    std::cout << "Test 2: Branchless Conditional Computation\n";
    std::cout << "Compares branchy arithmetic against branchless arithmetic.\n\n";

    run_benchmark("Branchy arithmetic", iterations, [&]() -> std::uint64_t {
        std::uint64_t sum = 0;

        for (int i = 0; i < n; i++) {
            int value = unpredictable[i];

            if (value < threshold) {
                sum += static_cast<std::uint64_t>(value * 3 + 1);
            } else {
                sum += static_cast<std::uint64_t>(value * 7 + 2);
            }
        }

        return sum;
    });

    run_benchmark("Branchless arithmetic", iterations, [&]() -> std::uint64_t {
        std::uint64_t sum = 0;

        for (int i = 0; i < n; i++) {
            int value = unpredictable[i];

            std::uint64_t true_case = static_cast<std::uint64_t>(value * 3 + 1);
            std::uint64_t false_case = static_cast<std::uint64_t>(value * 7 + 2);
            std::uint64_t condition = static_cast<std::uint64_t>(value < threshold);

            sum += condition * true_case + (1 - condition) * false_case;
        }

        return sum;
    });
}