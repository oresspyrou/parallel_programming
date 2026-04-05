//Headers
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

/**
 * @brief The function to integrate — with artificial non-uniform load.
 *
 * Computes sin(x) and then runs a variable number of extra iterations
 * depending on x, simulating non-uniform computational cost per node.
 * This stresses the dynamic scheduler: nodes with larger x are more expensive,
 * and the work queue ensures idle threads always pick up the next chunk.
 *
 * @param x  Variable value
 * @return   Approximate sin(x) with small numerical perturbation
 */
double f(double x) {
    double result = sin(x);
    // Artificial load: proportional to x — larger x costs more
    int iterations = (int)(x * 1000) % 500;
    for (int k = 0; k < iterations; k++)
        result += sin(result * 0.0001);
    return result;
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
    double a;          // lower bound (to compute a + i*h)
    double h;          // step width — computed in main
    int    n;          // total number of trapezoids — upper loop bound
    int    chunk_size; // how many nodes each thread processes per iteration
};

/**
 * @brief Thread function — dynamic scheduling with non-uniform workload.
 *
 * Repeatedly claims the next chunk from next_i, computes the local partial
 * sum, and adds it to global_sum. Stops when no more nodes remain.
 * The non-uniform f(x) means some chunks take longer than others;
 * dynamic scheduling ensures no thread stays idle while work remains.
 *
 * @param arg  Pointer to ThreadArgs
 * @return     NULL
 */
void* thread_func(void* arg) {
    ThreadArgs* t = (ThreadArgs*) arg;

    while (true) {
        // [LOCK] claim the next chunk and advance the counter
        pthread_mutex_lock(&mutex);
        int i_start = next_i;
        next_i += t->chunk_size;
        pthread_mutex_unlock(&mutex);

        // No more work available
        if (i_start >= t->n) break;

        // Compute chunk [i_start, i_end)
        int i_end = i_start + t->chunk_size;
        if (i_end > t->n) i_end = t->n;  // clamp last chunk to n

        double local_sum = 0.0;
        for (int i = i_start; i < i_end; i++) {
            local_sum += f(t->a + i * t->h);  // x_i = a + i*h
        }

        // [LOCK] add to global_sum
        pthread_mutex_lock(&mutex);
        global_sum += 2.0 * local_sum;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/**
 * @brief Main program. Reads a, b, n, num_threads, chunk_size from command line,
 *        dynamically distributes nodes across threads using a work queue,
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

    // All threads share the same args — distribution happens dynamically via next_i
    for (int i = 0; i < num_threads; i++) {
        args[i].a          = a;
        args[i].h          = h;
        args[i].n          = n;
        args[i].chunk_size = chunk_size;
    }

    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // Launch all threads
    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, thread_func, &args[i]);

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &te);

    // Combine endpoints (counted once) with the shared sum from all threads
    double total  = f(a) + f(b) + global_sum;
    double result = (h / 2.0) * total;

    double elapsed = (te.tv_sec - ts.tv_sec) + (te.tv_nsec - ts.tv_nsec) / 1e9;

    printf("Integral from %.4f to %.4f with n=%d, threads=%d, chunk=%d: %.10f\n", a, b, n, num_threads, chunk_size, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
