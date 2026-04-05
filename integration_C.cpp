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

/** @brief Mutex protecting both next_i (work counter) and global_sum. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/** @brief Shared variable where threads accumulate their partial sums. */
double global_sum = 0.0;

/** @brief Next node index to be processed — threads claim chunks from here dynamically. */
int next_i = 1;

/**
 * @brief Arguments passed by main to each thread.
 */
struct ThreadArgs {
    double a;          // lower bound (used to compute x_i = a + i*h)
    double h;          // step width — computed in main
    int    n;          // total number of trapezoids — upper loop bound
    int    chunk_size; // number of nodes each thread claims per iteration
};

/**
 * @brief Thread function — dynamic scheduling via shared work queue.
 *
 * Each thread repeatedly claims the next chunk of work from next_i,
 * computes the local partial sum independently, then adds it to global_sum.
 * Stops when no more nodes remain (next_i >= n).
 * Two separate critical sections are used to minimize lock contention:
 *   1. Claim the next chunk (short, just read + increment next_i)
 *   2. Add result to global_sum (short, just one addition)
 *
 * @param arg  Pointer to ThreadArgs
 * @return     NULL
 */
void* thread_func(void* arg) {
    ThreadArgs* t = (ThreadArgs*) arg;

    while (true) {
        // [LOCK] claim the next chunk and advance the work counter
        pthread_mutex_lock(&mutex);
        int i_start = next_i;
        next_i += t->chunk_size;
        pthread_mutex_unlock(&mutex);

        // No more work available
        if (i_start >= t->n) break;

        // Compute chunk [i_start, i_end) outside the lock
        int i_end = i_start + t->chunk_size;
        if (i_end > t->n) i_end = t->n;  // clamp last chunk to n

        double local_sum = 0.0;
        for (int i = i_start; i < i_end; i++) {
            local_sum += f(t->a + i * t->h);  // x_i = a + i*h
        }

        // [LOCK] add partial sum to the shared accumulator
        pthread_mutex_lock(&mutex);
        global_sum += 2.0 * local_sum;  // interior nodes count twice
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/**
 * @brief Main program. Reads a, b, n, num_threads, chunk_size from command line,
 *        dynamically distributes nodes across threads via a shared work queue,
 *        and prints the result and execution time.
 */
int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s a b n num_threads chunk_size\n", argv[0]);
        return 1;
    }

    double a           = atof(argv[1]);
    double b           = atof(argv[2]);
    int    n           = atoi(argv[3]);
    int    num_threads = atoi(argv[4]);
    int    chunk_size  = atoi(argv[5]);

    // Validate input
    if (n <= 0 || num_threads <= 0 || chunk_size <= 0) {
        fprintf(stderr, "Error: n, num_threads and chunk_size must be positive\n");
        return 1;
    }

    double h = (b - a) / n;  // width of each trapezoid

    ThreadArgs args[num_threads];
    pthread_t  threads[num_threads];

    // All threads share the same args — work distribution happens dynamically via next_i
    for (int i = 0; i < num_threads; i++) {
        args[i].a          = a;
        args[i].h          = h;
        args[i].n          = n;
        args[i].chunk_size = chunk_size;
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

    printf("Integral from %.4f to %.4f with n=%d, threads=%d, chunk=%d: %.10f\n", a, b, n, num_threads, chunk_size, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
