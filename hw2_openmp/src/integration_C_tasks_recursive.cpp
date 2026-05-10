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

// Recursively splits [i_start, i_end) into two halves, spawning a new task
// for each half, until the segment is smaller than min_size — at that point
// the segment is computed directly and its result added to sum.
// min_size acts as the grain size control: too small causes task explosion,
// too large reduces parallelism.
void integrate_recursive(double a, double h, int i_start, int i_end,
                         int min_size, double& sum) {
    if (i_end - i_start <= min_size) {
        // Base case: compute this segment directly
        double local = 0.0;
        for (int i = i_start; i < i_end; i++)
            local += f(a + i * h);
        #pragma omp critical
        sum += local;
        return;
    }

    // Recursive case: split in half and spawn two child tasks
    int mid = (i_start + i_end) / 2;

    #pragma omp task firstprivate(i_start, mid)
    integrate_recursive(a, h, i_start, mid, min_size, sum);

    #pragma omp task firstprivate(mid, i_end)
    integrate_recursive(a, h, mid, i_end, min_size, sum);

    // Wait for both child tasks to finish before returning
    #pragma omp taskwait
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
        // One thread kicks off the root task; all threads execute the tree
        #pragma omp single
        integrate_recursive(a, h, 1, n, min_size, sum);
    }

    double elapsed = omp_get_wtime() - start;

    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";

    return 0;
}
