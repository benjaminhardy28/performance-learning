#include "bench.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

struct ParticleAoS {
    float x;
    float y;
    float z;
    float velocity;

    float mass;
    float temperature;
    float pressure;
    float density;
    float color_r;
    float color_g;
    float color_b;
    float lifetime;
    float padding1;
    float padding2;
    float padding3;
    float padding4;
};

struct ParticlesSoA {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    std::vector<float> velocity;
};

void run_aos_vs_soa() {
    constexpr int n = 10'000'000;
    constexpr int iterations = 30;

    std::vector<ParticleAoS> aos(n);

    ParticlesSoA soa;
    soa.x.resize(n);
    soa.y.resize(n);
    soa.z.resize(n);
    soa.velocity.resize(n);

    for (int i = 0; i < n; i++) {
        float value = static_cast<float>(i % 100);

        aos[i] = ParticleAoS{
            value,
            value + 1.0f,
            value + 2.0f,
            value + 3.0f
        };

        soa.x[i] = value;
        soa.y[i] = value + 1.0f;
        soa.z[i] = value + 2.0f;
        soa.velocity[i] = value + 3.0f;
    }

    std::cout << "AoS vs SoA Memory Layout Benchmark\n";
    std::cout << "Elements: " << n << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Access pattern: sum only the x field\n\n";

    run_benchmark("Array of Structs", iterations, [&]() -> std::uint64_t {
        float sum = 0.0f;

        for (int i = 0; i < n; i++) {
            sum += aos[i].x;
        }

        return static_cast<std::uint64_t>(sum);
    });

    run_benchmark("Struct of Arrays", iterations, [&]() -> std::uint64_t {
        float sum = 0.0f;

        for (int i = 0; i < n; i++) {
            sum += soa.x[i];
        }

        return static_cast<std::uint64_t>(sum);
    });
}