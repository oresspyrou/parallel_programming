#include <iostream>
#include <cmath>
#include <omp.h>

// Non-uniform cost function: larger x values require more iterations,
// producing the irregular workload that makes task-based parallelism relevant.
double f(double x) {
    double r = sin(x);
    int iters = (int)(fabs(x) * 50) + 1;
    for (int k = 0; k < iters; k++)
        r += sin(r * 0.0001);
    return r;
}

// Computes the trapezoidal sum for a sub-interval [x_start, x_end) of indices.
// Called inside each task — no synchronisation needed on local_sum.
double integrate_segment(double a, double h, int i_start, int i_end) {
    double local_sum = 0.0;
    for (int i = i_start; i < i_end; i++)
        local_sum += f(a + i * h);
    return local_sum;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " a b n num_threads num_tasks\n";
        return 1;
    }

    double a           = atof(argv[1]);
    double b           = atof(argv[2]);
    int    n           = atoi(argv[3]);
    int    num_threads = atoi(argv[4]);
    int    num_tasks   = atoi(argv[5]);

    if (n <= 0 || num_threads <= 0 || num_tasks <= 0) {
        std::cerr << "Error: n, num_threads and num_tasks must be positive\n";
        return 1;
    }

    omp_set_num_threads(num_threads);

    double h   = (b - a) / n;
    double sum = 0.0;

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        // Only one thread creates the tasks; all threads execute them.
        #pragma omp single
        {
            int chunk = (n - 1) / num_tasks;

            for (int t = 0; t < num_tasks; t++) {
                int i_start = 1 + t * chunk;
                int i_end   = (t == num_tasks - 1) ? n : i_start + chunk;

                // Each task captures its segment boundaries and accumulates
                // into sum via a critical section — tasks run concurrently,
                // only the final addition is serialised.
                #pragma omp task firstprivate(i_start, i_end) shared(sum)
                {
                    double local = integrate_segment(a, h, i_start, i_end);
                    #pragma omp critical
                    sum += local;
                }
            }
        }
        // implicit taskwait at end of single; all tasks complete before barrier
    }

    double elapsed = omp_get_wtime() - start;

    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";

    return 0;
}
