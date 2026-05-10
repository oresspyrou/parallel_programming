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

// Computes the trapezoidal interior sum for indices [i_start, i_end).
double integrate_segment(double a, double h, int i_start, int i_end) {
    double local = 0.0;
    for (int i = i_start; i < i_end; i++)
        local += f(a + i * h);
    return local;
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

    double h = (b - a) / n;

    // Each task writes its result to its own slot — no synchronisation needed.
    double* partial = new double[num_tasks]();

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        // One thread creates all tasks; all threads in the team execute them.
        #pragma omp single
        {
            int chunk = (n - 1) / num_tasks;

            for (int t = 0; t < num_tasks; t++) {
                int i_start = 1 + t * chunk;
                int i_end   = (t == num_tasks - 1) ? n : i_start + chunk;

                // firstprivate ensures each task gets its own copies of the
                // loop indices; partial is shared (pointer, not the array).
                #pragma omp task firstprivate(t, i_start, i_end)
                partial[t] = integrate_segment(a, h, i_start, i_end);
            }

            // Wait for all tasks to finish before reading partial[].
            #pragma omp taskwait

            // Merge partial results in the thread that created the tasks.
        }
    }

    double sum = 0.0;
    for (int t = 0; t < num_tasks; t++)
        sum += partial[t];

    double elapsed = omp_get_wtime() - start;

    double integral = (h / 2.0) * (f(a) + f(b) + 2.0 * sum);

    std::cout << "Integral from " << a << " to " << b
              << " = " << integral << "\n";
    std::cout << "Execution time: " << elapsed << " seconds\n";

    delete[] partial;
    return 0;
}
