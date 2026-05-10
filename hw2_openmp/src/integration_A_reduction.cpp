#include <iostream>
#include <cmath>
#include <omp.h>

// Function to integrate: sin(x)
double f(double x) {
    return sin(x);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " a b n num_threads\n";
        return 1;
    }

    double a           = atof(argv[1]);
    double b           = atof(argv[2]);
    int    n           = atoi(argv[3]);
    int    num_threads = atoi(argv[4]);

    if (n <= 0 || num_threads <= 0) {
        std::cerr << "Error: n and num_threads must be positive\n";
        return 1;
    }

    omp_set_num_threads(num_threads);

    double h   = (b - a) / n;
    double sum = 0.0;

    double start = omp_get_wtime();

    // reduction(+:sum) gives each thread a private copy of sum initialised to 0.
    // Threads accumulate independently with no synchronisation overhead.
    // OpenMP merges all private copies into the shared sum at the end of the region.
    #pragma omp parallel for reduction(+:sum)
    for (int i = 1; i < n; i++) {
        sum += f(a + i * h);
    }

    double elapsed = omp_get_wtime() - start;

    // Trapezoidal rule: h/2 * (f(a) + f(b) + 2 * sum_interior)
    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";

    return 0;
}
