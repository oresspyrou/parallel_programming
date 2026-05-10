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
    double sum = 0.0;  // accumulates interior node values

    double start = omp_get_wtime();

    // Each thread adds its result directly to the shared sum using a critical
    // section — only one thread can update sum at a time, causing contention
    // on every single iteration. This serializes the accumulation and degrades
    // performance as thread count grows.
    #pragma omp parallel for
    for (int i = 1; i < n; i++) {
        double val = f(a + i * h);
        #pragma omp critical
        sum += val;
    }

    double elapsed = omp_get_wtime() - start;

    // Trapezoidal rule: h/2 * (f(a) + f(b) + 2 * sum_interior)
    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";

    return 0;
}
