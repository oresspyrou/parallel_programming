// Headers
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/**
 * @brief The function to integrate.
 * @param x  Variable value
 * @return   sin(x)
 */
double f(double x) {
    return sin(x);
}

/**
 * @brief Computes the integral of f(x) over [a,b] using the trapezoidal rule.
 *
 * Divides [a,b] into n equal segments and sums the areas of the trapezoids.
 * Formula: (h/2) * [f(a) + f(b) + 2*sum(f(a+i*h), i=1..n-1)]
 *
 * @param a  Lower bound of the interval
 * @param b  Upper bound of the interval
 * @param n  Number of trapezoids
 * @return   Approximate value of the integral
 */
double trapezoidal(double a, double b, int n) {
    double h   = (b - a) / n;       // width of each trapezoid
    double sum = f(a) + f(b);       // endpoints count once

    for (int i = 1; i < n; i++) {
        sum += 2.0 * f(a + i * h);  // interior nodes count twice
    }

    return (h / 2.0) * sum;
}

/**
 * @brief Main program. Reads a, b, n from command line and prints
 *        the integral result along with execution time.
 */
int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s a b n\n", argv[0]);
        return 1;
    }

    double a = atof(argv[1]);
    double b = atof(argv[2]);
    int    n = atoi(argv[3]);

    // Validate input: n must be a positive integer
    if (n <= 0) {
        fprintf(stderr, "Error: n must be positive\n");
        return 1;
    }

    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);  // start timer

    double result = trapezoidal(a, b, n);

    clock_gettime(CLOCK_MONOTONIC, &te);  // stop timer
    double elapsed = (te.tv_sec - ts.tv_sec) + (te.tv_nsec - ts.tv_nsec) / 1e9;

    printf("Integral from %.4f to %.4f with n=%d: %.10f\n", a, b, n, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
