# AoS vs SoA Summary: One Cache-Line Structs

In this benchmark, the Array of Structs layout was modified so that each particle struct contains 16 float fields.

Since each float is 4 bytes:

16 floats × 4 bytes = 64 bytes

A typical CPU cache line is 64 bytes, so each ParticleAoS object is approximately one full cache line.

## AoS Layout

The AoS struct looked like this:

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

Memory is laid out like this:

[x y z velocity mass temperature pressure density color_r color_g color_b lifetime padding1 padding2 padding3 padding4]
[x y z velocity mass temperature pressure density color_r color_g color_b lifetime padding1 padding2 padding3 padding4]
[x y z velocity mass temperature pressure density color_r color_g color_b lifetime padding1 padding2 padding3 padding4]

The benchmark only summed the x field:

sum += particles[i].x;

Because each struct is about 64 bytes, reading one particle likely brings one full cache line into cache. However, only the first 4 bytes, the x field, are actually useful for this benchmark.

That means each cache line contains:

Useful data:
x

Unused data:
y, z, velocity, mass, temperature, pressure, density,
color_r, color_g, color_b, lifetime,
padding1, padding2, padding3, padding4

So AoS wastes most of the cache line for this access pattern.

## SoA Layout

In the Struct of Arrays layout, the x values are stored together:

x x x x x x x x x x x x x x x x

Since each float is 4 bytes and a cache line is about 64 bytes, one cache line can hold:

64 bytes / 4 bytes = 16 x values

So when the benchmark reads from the SoA x array, each cache line contains mostly useful data.

## Result

With one-cache-line AoS structs, SoA became clearly faster.

Across the stable runs:

Layout: Array of Structs
Average Total Time: 310,619,240 ns

Layout: Struct of Arrays
Average Total Time: 283,245,281 ns

The speedup was:

310,619,240 / 283,245,281 ≈ 1.097x

So the SoA layout was approximately 1.10x faster, or about 9.7% faster.

## Takeaway

When each AoS object occupies about one cache line and the loop only reads one field, AoS wastes cache bandwidth. The CPU loads a full 64-byte particle, but only the 4-byte x field is used.

SoA avoids this waste by storing all x values contiguously, allowing each cache line to provide about 16 useful values instead of just one.

The key idea is:

SoA becomes more beneficial when each object has many fields but the hot loop only needs a small subset of them.

This benchmark shows why data-oriented layout can improve performance in tight loops even when the algorithm itself does not change.