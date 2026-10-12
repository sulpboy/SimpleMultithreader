# SimpleMultithreader

> A lightweight, header-only C++11 library that simplifies parallel loop execution using POSIX threads (Pthreads) and lambda expressions.

## Overview

SimpleMultithreader abstracts the boilerplate involved in parallel programming, including thread creation, workload distribution, synchronization, and cleanup. It provides overloaded `parallel_for` APIs for one-dimensional and two-dimensional loops, allowing programmers to parallelize computations without manually managing Pthreads.

The library creates a fresh set of threads for each invocation and uses the calling thread as one of the workers, without relying on `std::thread`, `std::async`, or thread pools.

## Features

- **1D Parallel Loops:** Distributes iterations of a standard loop across multiple workers.
- **2D Parallel Loops:** Parallelizes the outer loop of a nested iteration while processing the inner loop sequentially.
- **Pthread-Based Execution:** Uses `pthread_create()` and `pthread_join()` to manage thread creation and synchronization.
- **Lambda-Based API:** Accepts C++11 lambda expressions through `std::function`, keeping application code concise.
- **Execution-Time Measurement:** Reports the elapsed time of each `parallel_for` invocation in microseconds using `CLOCK_MONOTONIC`.

The specified thread count includes the calling thread.

## Project Structure

```text
SimpleMultithreader/
├── simple-multithreader.h   # Header-only library implementation
├── vector.cpp               # Parallel vector addition
├── matrix.cpp               # Parallel matrix multiplication
├── Makefile                 # Build configuration
└── README.md
```

## Getting Started

### Prerequisites

- C++11-compatible compiler
- POSIX threads (Pthreads)
- Unix-like environment, such as Linux or macOS

### Compilation

Compile the sample programs using `g++`:

```bash
g++ -std=c++11 -O2 -pthread vector.cpp -o vector
g++ -std=c++11 -O2 -pthread matrix.cpp -o matrix
```

### Running the Examples

**Vector addition**

```bash
./vector 4 1000000
```

Runs vector addition on 1,000,000 elements using 4 threads.

**Matrix multiplication**

```bash
./matrix 4 128
```

Multiplies two 128 × 128 matrices using 4 threads.

Both examples verify their results using assertions and print a success message when the checks pass. Each `parallel_for` invocation also reports its execution time.

## API Reference

### One-Dimensional Loop

```cpp
void parallel_for(
    int low,
    int high,
    std::function<void(int)>&& lambda,
    int numThreads
);
```

Executes loop iterations over the half-open interval `[low, high)` in parallel.

### Two-Dimensional Loop

```cpp
void parallel_for(
    int low1,
    int high1,
    int low2,
    int high2,
    std::function<void(int, int)>&& lambda,
    int numThreads
);
```

Parallelizes the outer iteration range `[low1, high1)` and executes the inner range `[low2, high2)` for each outer iteration.

## Implementation Highlights

- **Thread lifecycle:** Creates threads for each invocation and joins them before returning.
- **Work partitioning:** Divides the iteration range into chunks assigned to workers.
- **Synchronization:** Ensures all worker threads finish before execution proceeds beyond the call.
- **Timing:** Measures elapsed wall-clock time, including thread creation, computation, and joining.

## Learning Outcomes

This project explores Pthread programming, parallel loop decomposition, C++11 lambda expressions, thread synchronization, and execution-time measurement.

---

*Built with C++11 and POSIX threads.*
