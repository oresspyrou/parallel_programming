#include <iostream>
#include <cmath>
#include <omp.h>

// Non-uniform cost function: iterations scale with x, so larger i values
// require more work. This creates load imbalance that exposes scheduling differences.
double f(double x) {
    double r = sin(x);
    int iters = (int)(fabs(x) * 50) + 1;
    for (int k = 0; k < iters; k++)
        r += sin(r * 0.0001);
    return r;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " a b n num_threads chunk_size\n";
        return 1;
    }

    double a           = atof(argv[1]);
    double b           = atof(argv[2]);
    int    n           = atoi(argv[3]);
    int    num_threads = atoi(argv[4]);
    int    chunk_size  = atoi(argv[5]);

    if (n <= 0 || num_threads <= 0 || chunk_size <= 0) {
        std::cerr << "Error: n, num_threads and chunk_size must be positive\n";
        return 1;
    }

    omp_set_num_threads(num_threads);

    double h   = (b - a) / n;
    double sum = 0.0;

    double start = omp_get_wtime();

    // static scheduling: the iteration space is divided into chunks of chunk_size
    // and assigned to threads round-robin before execution begins.
    // With a non-uniform f(x), threads that receive high-x chunks finish later,
    // leaving other threads idle — this is the load imbalance we are measuring.
    #pragma omp parallel for reduction(+:sum) schedule(static, chunk_size)
    for (int i = 1; i < n; i++) {
        sum += f(a + i * h);
    }

    double elapsed = omp_get_wtime() - start;

    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";

    return 0;
}
