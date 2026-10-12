#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <exception>

static inline int64_t cur_tim() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000LL + (ts.tv_nsec / 1000);
}

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

// --------------------- 1D parallel_for ---------------------
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
    if (numThreads <= 0) numThreads = 1;


    if (low >= high) {
        int64_t t0 = cur_tim();
        int64_t t1 = cur_tim();
        printf("[SimpleMultithreader] parallel_for(1D) trivial: %lld us (no iterations)\n", (long long)(t1 - t0));
        return;
    }

    std::function<void(int)> func = std::move(lambda);

    const long long totalIter = (long long)high - (long long)low;
    const int numT = numThreads;

    const long long chunksiz = (totalIter + numT - 1) / numT;

    struct Thread_1 {
        int tid;
        int low;
        int high;
        long long chunksiz;
        std::function<void(int)> *func;
    };

    std::vector<Thread_1> args(numT);
    std::vector<pthread_t> threads;
    threads.reserve((size_t)std::max(0, numT - 1));

    for (int t = 0; t < numT; ++t) {
        args[t].tid = t;
        args[t].low = low;
        args[t].high = high;
        args[t].chunksiz = chunksiz;
        args[t].func = &func;
    }

    auto task = [](void *v) -> void* {
        Thread_1 *a = static_cast<Thread_1*>(v);
        int tid = a->tid;
        long long start = (long long)a->low + (long long)tid * a->chunksiz;
        long long end = start + a->chunksiz;
        if (start < a->low) start = a->low;
        if (end > a->high) end = a->high;
        std::function<void(int)> &f = *(a->func);

        try {
            for (long long i = start; i < end; ++i) {
                f((int)i);
            }
        } catch (const std::exception &e) {
            fprintf(stderr, "[SimpleMultithreader] thread %d exception: %s\n", tid, e.what());
            abort();
        } catch (...) {
            fprintf(stderr, "[SimpleMultithreader] thread %d unknown exception\n", tid);
            abort();
        }
        return nullptr;
    };

    int64_t tstart = cur_tim();

    for (int t = 1; t < numT; ++t) {
        pthread_t thr;
        int rc = pthread_create(&thr, nullptr, task, (void*)&args[t]);
        if (rc != 0) {
            fprintf(stderr, "[SimpleMultithreader] pthread_create failed: %s\n", strerror(rc));
            abort();
        }
        threads.push_back(thr);
    }

    task((void*)&args[0]);

    // join others
    for (size_t i = 0; i < threads.size(); ++i) {
        int rc = pthread_join(threads[i], nullptr);
        if (rc != 0) {
            fprintf(stderr, "[SimpleMultithreader] pthread_join failed: %s\n", strerror(rc));
            abort();
        }
    }

    int64_t tend = cur_tim();
    printf("[SimpleMultithreader] parallel_for(1D) time: %lld us\n", (long long)(tend - tstart));
}

// 2D parallel_for
void parallel_for(int low1, int high1, int low2, int high2,
                  std::function<void(int,int)> &&lambda, int numThreads) {
    if (numThreads <= 0) numThreads = 1;

    if (low1 >= high1 || low2 >= high2) {
        int64_t t0 = cur_tim();
        int64_t t1 = cur_tim();
        printf("[SimpleMultithreader] parallel_for(2D) trivial: %lld us (no iterations)\n", (long long)(t1 - t0));
        return;
    }

    std::function<void(int,int)> func = std::move(lambda);

    const long long outerIter = (long long)high1 - (long long)low1;
    const int numT = numThreads;
    const long long chunksiz = (outerIter + numT - 1) / numT;

    struct Thread_2 {
        int tid;
        int low1;
        int high1;
        int low2;
        int high2;
        long long chunksiz;
        std::function<void(int,int)> *func;
    };

    std::vector<Thread_2> args(numT);
    std::vector<pthread_t> threads;
    threads.reserve((size_t)std::max(0, numT - 1));

    for (int t = 0; t < numT; ++t) {
        args[t].tid = t;
        args[t].low1 = low1;
        args[t].high1 = high1;
        args[t].low2 = low2;
        args[t].high2 = high2;
        args[t].chunksiz = chunksiz;
        args[t].func = &func;
    }

    auto task22 = [](void *v) -> void* {
        Thread_2 *a = static_cast<Thread_2*>(v);
        int tid = a->tid;
        long long start_i = (long long)a->low1 + (long long)tid * a->chunksiz;
        long long end_i = start_i + a->chunksiz;
        if (start_i < a->low1) start_i = a->low1;
        if (end_i > a->high1) end_i = a->high1;
        std::function<void(int,int)> &f = *(a->func);

        try {
            for (long long i = start_i; i < end_i; ++i) {
                for (int j = a->low2; j < a->high2; ++j) {
                    f((int)i, j);
                }
            }
        } catch (const std::exception &e) {
            fprintf(stderr, "[SimpleMultithreader] thread %d exception: %s\n", tid, e.what());
            abort();
        } catch (...) {
            fprintf(stderr, "[SimpleMultithreader] thread %d unknown exception\n", tid);
            abort();
        }
        return nullptr;
    };

    int64_t tstart = cur_tim();

    for (int t = 1; t < numT; ++t) {
        pthread_t thr;
        int rc = pthread_create(&thr, nullptr, task22, (void*)&args[t]);
        if (rc != 0) {
            fprintf(stderr, "[SimpleMultithreader] pthread_create failed: %s\n", strerror(rc));
            abort();
        }
        threads.push_back(thr);
    }

    task22((void*)&args[0]);

    for (size_t i = 0; i < threads.size(); ++i) {
        int rc = pthread_join(threads[i], nullptr);
        if (rc != 0) {
            fprintf(stderr, "[SimpleMultithreader] pthread_join failed: %s\n", strerror(rc));
            abort();
        }
    }

    int64_t tend = cur_tim();
    printf("[SimpleMultithreader] parallel_for(2D) time: %lld us\n", (long long)(tend - tstart));
}


#define main user_main
#endif // SIMPLE_MULTITHREADER_H
