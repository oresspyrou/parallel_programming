// Headers
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

/**
 * @brief The function to integrate.
 * @param x  Variable value
 * @return   sin(x)
 */
double f(double x) {
    return sin(x);
}

/** @brief Mutex protecting global_sum from race conditions. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/** @brief Shared variable where threads accumulate their partial sums. */
double global_sum = 0.0;

/**
 * @brief Arguments passed by main to each thread.
 */
struct ThreadArgs {
    int    start;  // first index i this thread processes (inclusive)
    int    end;    // last index i this thread processes (exclusive)
    double a;      // lower bound (used to compute x_i = a + i*h)
    double h;      // step width — computed in main
};

/**
 * @brief Thread function — static block distribution with mutex lock.
 *
 * Each thread computes its local partial sum for nodes in [start, end),
 * then acquires the mutex to safely add the result to global_sum.
 * Lock contention is minimal: each thread locks only once after its computation.
 *
 * @param arg  Pointer to ThreadArgs
 * @return     NULL
 */
void* thread_func(void* arg) {
    ThreadArgs* t = (ThreadArgs*) arg;

    double local_sum = 0.0;
    for (int i = t->start; i < t->end; i++) {
        local_sum += f(t->a + i * t->h);  // x_i = a + i*h
    }

    // Critical section: add partial sum to the shared accumulator
    pthread_mutex_lock(&mutex);
    global_sum += 2.0 * local_sum;  // interior nodes count twice
    pthread_mutex_unlock(&mutex);

    return NULL;
}

/**
 * @brief Main program. Reads a, b, n, num_threads from command line,
 *        distributes work in contiguous blocks using a mutex-protected global_sum,
 *        and prints the result and execution time.
 */
int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s a b n num_threads\n", argv[0]);
        return 1;
    }

    double a           = atof(argv[1]);
    double b           = atof(argv[2]);
    int    n           = atoi(argv[3]);
    int    num_threads = atoi(argv[4]);

    // Validate input: n and num_threads must be positive
    if (n <= 0 || num_threads <= 0) {
        fprintf(stderr, "Error: n and num_threads must be positive\n");
        return 1;
    }

    double h = (b - a) / n;  // width of each trapezoid

    ThreadArgs args[num_threads];
    pthread_t  threads[num_threads];

    // Distribute interior nodes i=1..n-1 across threads in contiguous blocks
    // Endpoints f(a) and f(b) are handled by main after all threads finish
    int chunk = (n - 1) / num_threads;
    for (int i = 0; i < num_threads; i++) {
        args[i].start = 1 + i * chunk;
        args[i].end   = (i == num_threads - 1) ? n : 1 + (i + 1) * chunk;
        args[i].a     = a;
        args[i].h     = h;
    }

    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);  // start timer

    // Launch all threads
    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, thread_func, &args[i]);

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &te);  // stop timer

    // Combine endpoints (counted once) with the shared sum from all threads
    double total  = f(a) + f(b) + global_sum;
    double result = (h / 2.0) * total;

    double elapsed = (te.tv_sec - ts.tv_sec) + (te.tv_nsec - ts.tv_nsec) / 1e9;

    printf("Integral from %.4f to %.4f with n=%d, threads=%d: %.10f\n", a, b, n, num_threads, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
