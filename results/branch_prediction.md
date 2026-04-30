# Branch Prediction and Branchless Computation Benchmark Summary

## What This Benchmark Tested

This experiment tested two related CPU performance concepts:

1. Branch prediction
2. Branchless conditional computation

The goal was to understand how conditional logic affects performance in hot loops.

The benchmark was split into two separate tests:

- Test 1: Predictable branch vs unpredictable branch using noinline function calls
- Test 2: Branchy arithmetic vs branchless arithmetic

Splitting the benchmark into two tests made the results easier to interpret.

## Background

Modern CPUs use branch prediction to guess which direction a branch will take before the condition is fully resolved.

For example:

if (value < threshold) {
    ...
} else {
    ...
}

If the CPU predicts correctly, execution continues efficiently.

If the CPU predicts incorrectly, the CPU may have already started executing the wrong path. It then has to discard that speculative work and continue from the correct path. This is called a branch misprediction, and it can be expensive in tight loops.

## Test Setup

The benchmark used:

- Elements: 10,000,000
- Iterations: 30
- Threshold: 128
- Values: integers in the range 0 to 255

Because the threshold was 128, about half the values were below the threshold and half were above it.

This created two possible branch outcomes:

value < 128  -> true
value >= 128 -> false

## Test 1: Branch Prediction Isolation

### What It Compared

Test 1 compared:

1. Predictable branch with noinline calls
2. Unpredictable branch with noinline calls

The predictable data was sorted, so the branch pattern looked like:

true true true true true ... false false false false false

The unpredictable data was shuffled, so the branch pattern looked more like:

true false true true false false true false ...

### Why noinline Functions Were Used

An earlier version of the benchmark used simple arithmetic directly inside the branch:

if (value < threshold) {
    sum += value * 3 + 1;
} else {
    sum += value * 7 + 2;
}

With compiler optimizations enabled, this was too easy for the compiler to optimize. The compiler could potentially transform the branch into branchless instructions, conditional selects, or vectorized operations.

That made it difficult to isolate the cost of branch prediction.

To make the branch harder to optimize away, the benchmark used noinline functions:

NOINLINE std::uint64_t true_path(int value) {
    return static_cast<std::uint64_t>(value * 3 + 1);
}

NOINLINE std::uint64_t false_path(int value) {
    return static_cast<std::uint64_t>(value * 7 + 2);
}

Then the benchmark loop became:

if (value < threshold) {
    sum += true_path(value);
} else {
    sum += false_path(value);
}

The noinline attribute tells the compiler not to inline these functions into the loop. This makes the branch more visible in the generated code because the branch controls which function call happens.

This helped isolate the performance impact of predictable versus unpredictable control flow.

## Test 1 Results

| Benchmark | Total Time | Average Time Per Iteration |
|---|---:|---:|
| Predictable Branch with noinline calls | 282,960,334 ns | 9,432,011 ns |
| Unpredictable Branch with noinline calls | 1,140,270,458 ns | 38,009,015 ns |

### Test 1 Speedup

Unpredictable / Predictable:

1,140,270,458 / 282,960,334 ≈ 4.03x

The predictable branch was approximately 4.03x faster than the unpredictable branch.

## Test 1 Interpretation

The predictable branch was much faster because the CPU could learn the branch pattern.

In the predictable case, the values were sorted. The condition was true for a long stretch, then false for a long stretch. This is easy for the branch predictor to handle.

In the unpredictable case, the values were shuffled. The branch outcome changed randomly, making it harder for the branch predictor to guess correctly.

The large slowdown in the unpredictable version shows the cost of branch mispredictions when the branch cannot be optimized away.

## Test 2: Branchless Conditional Computation

### What It Compared

Test 2 compared:

1. Branchy arithmetic
2. Branchless arithmetic

The branchy version used an if/else directly in the loop:

if (value < threshold) {
    sum += static_cast<std::uint64_t>(value * 3 + 1);
} else {
    sum += static_cast<std::uint64_t>(value * 7 + 2);
}

The branchless version computed both possible values and selected the correct one using arithmetic:

std::uint64_t true_case = static_cast<std::uint64_t>(value * 3 + 1);
std::uint64_t false_case = static_cast<std::uint64_t>(value * 7 + 2);
std::uint64_t condition = static_cast<std::uint64_t>(value < threshold);

sum += condition * true_case + (1 - condition) * false_case;

If the condition is true, condition is 1 and the true case is selected.

If the condition is false, condition is 0 and the false case is selected.

This avoids explicit branch control flow in the source code.

## Test 2 Results

| Benchmark | Total Time | Average Time Per Iteration |
|---|---:|---:|
| Branchy arithmetic | 219,089,875 ns | 7,302,996 ns |
| Branchless arithmetic | 65,602,584 ns | 2,186,753 ns |

### Test 2 Speedup

Branchy / Branchless:

219,089,875 / 65,602,584 ≈ 3.34x

The branchless arithmetic version was approximately 3.34x faster than the branchy arithmetic version.

## Test 2 Interpretation

The branchless version was significantly faster because it removed the conditional branch from the hot loop.

For simple arithmetic, branchless code can be easier for the compiler to optimize. It may allow better instruction scheduling, conditional-select style instructions, or vectorization.

This does not mean branchless code is always faster. Branchless code can do extra work because it may compute both possible outcomes. It is most useful when:

- the branch is hard to predict
- the branch body is simple
- the extra arithmetic is cheaper than branch mispredictions
- the compiler can optimize the straight-line computation well

## Main Takeaways

This experiment demonstrated two different lessons.

### Lesson 1: Branch prediction matters

When the branch was made harder to optimize away using noinline function calls, predictable control flow was much faster than unpredictable control flow.

The predictable branch was about 4.03x faster than the unpredictable branch.

This shows that branch mispredictions can be very expensive in tight loops.

### Lesson 2: Branchless code can help

When the conditional work was simple arithmetic, the branchless version was much faster than the branchy version.

The branchless arithmetic version was about 3.34x faster.

This shows that removing branches can sometimes allow the compiler and CPU to execute a hot loop more efficiently.

## Important Benchmarking Note

This benchmark also showed why benchmark design matters.

A simple source-level if statement does not always mean the final machine code contains an expensive branch. With optimization flags like -O3, the compiler may transform simple branches into branchless or vectorized code.

The noinline functions helped prevent the compiler from hiding the branch cost, making the branch prediction effect easier to observe.

## Conclusion

This experiment showed that conditional logic can have a major impact on performance.

The noinline branch test demonstrated the cost of unpredictable branches, while the branchless arithmetic test demonstrated how removing a branch can improve performance for simple conditional computations.

The broader lesson is:

Branch performance depends on predictability, compiler optimization, and the amount of work inside each branch.

For performance-critical code, it is important to measure rather than assume.
