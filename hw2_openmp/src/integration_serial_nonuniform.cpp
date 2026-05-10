#include <iostream>
#include <cmath>
#include <chrono>
#include <cstdlib>

// Same non-uniform cost function used in B and C programs.
// Serves as the serial baseline for scheduling and task experiments.
double f(double x) {
    double r = sin(x);
    int iters = (int)(fabs(x) * 50) + 1;
    for (int k = 0; k < iters; k++)
        r += sin(r * 0.0001);
    return r;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " a b n\n";
        return 1;
    }

    double a = atof(argv[1]);
    double b = atof(argv[2]);
    int    n = atoi(argv[3]);

    if (n <= 0) {
        std::cerr << "Error: n must be positive\n";
        return 1;
    }

    double h   = (b - a) / n;
    double sum = 0.0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 1; i < n; i++)
        sum += f(a + i * h);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed.count() << " seconds\n";

    return 0;
}
