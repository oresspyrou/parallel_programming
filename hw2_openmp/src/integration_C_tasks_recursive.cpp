#include <iostream>
#include <cmath>
#include <omp.h>

// Non-uniform cost function: larger x values require more iterations,
// producing the irregular workload suited for recursive task decomposition.
double f(double x) {
    double r = sin(x);
    int iters = (int)(fabs(x) * 50) + 1;
    for (int k = 0; k < iters; k++)
        r += sin(r * 0.0001);
    return r;
}

// Recursively splits [i_start, i_end) in half, spawning a child task for each
// half, until the segment is smaller than min_size — then computes directly.
// Returns the interior sum for its segment so no shared accumulator is needed.
// min_size controls task granularity: too small causes task explosion and
// excessive overhead; too large reduces available parallelism.
double integrate_recursive(double a, double h, int i_start, int i_end, int min_size) {
    if (i_end - i_start <= min_size) {
        double local = 0.0;
        for (int i = i_start; i < i_end; i++)
            local += f(a + i * h);
        return local;
    }

    int    mid   = (i_start + i_end) / 2;
    double left  = 0.0;
    double right = 0.0;

    #pragma omp task shared(left) firstprivate(i_start, mid)
    left = integrate_recursive(a, h, i_start, mid, min_size);

    #pragma omp task shared(right) firstprivate(mid, i_end)
    right = integrate_recursive(a, h, mid, i_end, min_size);

    // Wait for both child tasks before combining their results.
    #pragma omp taskwait

    return left + right;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " a b n num_threads min_size\n";
        return 1;
    }

    double a           = atof(argv[1]);
    double b           = atof(argv[2]);
    int    n           = atoi(argv[3]);
    int    num_threads = atoi(argv[4]);
    int    min_size    = atoi(argv[5]);

    if (n <= 0 || num_threads <= 0 || min_size <= 0) {
        std::cerr << "Error: n, num_threads and min_size must be positive\n";
        return 1;
    }

    omp_set_num_threads(num_threads);

    double h   = (b - a) / n;
    double sum = 0.0;

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        // One thread starts the root task; all threads execute the task tree.
        #pragma omp single
        sum = integrate_recursive(a, h, 1, n, min_size);
    }

    double elapsed = omp_get_wtime() - start;

    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";

    return 0;
}
