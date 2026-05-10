// Headers
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
 * Nodes with larger x values require more work, creating load imbalance.
 * Cyclic distribution handles this better than block distribution because
 * it interleaves cheap and expensive nodes across all threads.
 *
 * @param x  Variable value
 * @return   Approximate sin(x) with small numerical perturbation
 */
double f(double x) {
    double result = sin(x);
    // Artificial load: number of iterations grows with x
    int iterations = (int)(x * 1000) % 500;
    for (int k = 0; k < iterations; k++)
        result += sin(result * 0.0001);
    return result;
}

/**
 * @brief Arguments passed by main to each thread.
 *
 * Uses cyclic (interleaved) distribution: thread k processes
 * nodes k+1, k+1+T, k+1+2T, ... where T = num_threads.
 */
struct ThreadArgs {
    int    thread_id;   // thread ID — determines which nodes this thread processes
    int    num_threads; // total number of threads — stride of the cyclic loop
    int    n;           // total number of trapezoids — upper loop bound
    double a;           // lower bound of integration
    double h;           // step width — computed in main
    double result;      // thread writes its partial sum here (no lock needed)
};

/**
 * @brief Thread function — cyclic (interleaved) distribution, no locks.
 *
 * Thread with id=k processes nodes: i = k+1, k+1+T, k+1+2T, ... (T = num_threads).
 * With the non-uniform f(x), cyclic distribution achieves better load balance
 * than block distribution because expensive nodes are spread across all threads.
 * No synchronization needed — each thread writes to its own result field.
 *
 * @param arg  Pointer to ThreadArgs
 * @return     NULL
 */
void* thread_func(void* arg) {
    ThreadArgs* t = (ThreadArgs*) arg;

    double local_sum = 0.0;
    for (int i = t->thread_id + 1; i < t->n; i += t->num_threads) {
        local_sum += f(t->a + i * t->h);  // x_i = a + i*h
    }

    // Interior nodes are multiplied by 2 in the trapezoidal formula
    t->result = 2.0 * local_sum;
    return NULL;
}

/**
 * @brief Main program. Reads a, b, n, num_threads from command line,
 *        distributes nodes using cyclic scheduling with non-uniform f(x),
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

    // Cyclic distribution: thread i handles nodes i+1, i+1+T, i+1+2T, ...
    // Endpoints f(a) and f(b) are handled by main after all threads finish
    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id   = i;
        args[i].num_threads = num_threads;
        args[i].n           = n;
        args[i].a           = a;
        args[i].h           = h;
        args[i].result      = 0.0;
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

    // Collect partial sums — endpoints f(a) and f(b) count once
    double total = f(a) + f(b);
    for (int i = 0; i < num_threads; i++)
        total += args[i].result;
    double result = (h / 2.0) * total;

    double elapsed = (te.tv_sec - ts.tv_sec) + (te.tv_nsec - ts.tv_nsec) / 1e9;

    printf("Integral from %.4f to %.4f with n=%d, threads=%d: %.10f\n", a, b, n, num_threads, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
