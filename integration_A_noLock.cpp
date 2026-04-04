//Headers
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

/**
 * @brief Η συνάρτηση προς ολοκλήρωση.
 * @param x  Τιμή της μεταβλητής
 * @return   sin(x)
 */
double f(double x) {
    return sin(x);
}

/**
 * @brief Ορίσματα που περνάει το main σε κάθε thread.
 */
struct ThreadArgs {
    int    start;    // πρώτο i που επεξεργάζεται
    int    end;      // τελευταίο i (exclusive)
    double a;        // κάτω όριο (για να υπολογίζει a + i*h)
    double h;        // βήμα — υπολογισμένο στο main
    double result;   // εδώ γράφει το μερικό άθροισμά του
};

/**
 * @brief Συνάρτηση που εκτελεί κάθε thread.
 *
 * Υπολογίζει το μερικό άθροισμα 2*f(x_i) για τους κόμβους [start, end)
 * και το αποθηκεύει στο args->result (χωρίς lock — κάθε thread γράφει στο δικό του).
 *
 * @param arg  Δείκτης σε ThreadArgs
 * @return     NULL
 */
void* thread_func(void* arg) {
    ThreadArgs* t = (ThreadArgs*) arg;

    double local_sum = 0.0;
    for (int i = t->start; i < t->end; i++) {
        local_sum += f(t->a + i * t->h);
    }

    t->result = 2.0 * local_sum;
    return NULL;
}

/**
 * @brief Κύριο πρόγραμμα. Διαβάζει a, b, n, num_threads από command line,
 *        μοιράζει τον υπολογισμό σε threads και εκτυπώνει αποτέλεσμα + χρόνο.
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

    double h = (b - a) / n;

    ThreadArgs args[num_threads];
    pthread_t  threads[num_threads];

    // Τα threads καλύπτουν μόνο i = 1 έως n-1 (τα άκρα f(a)+f(b) υπολογίζονται στο main)
    int chunk = (n - 1) / num_threads;
    for (int i = 0; i < num_threads; i++) {
        args[i].start  = 1 + i * chunk;
        args[i].end    = (i == num_threads - 1) ? n : 1 + (i + 1) * chunk;
        args[i].a      = a;
        args[i].h      = h;
        args[i].result = 0.0;
    }

    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, thread_func, &args[i]);

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &te);

    // Συγκέντρωσε μερικά αθροίσματα — τα άκρα f(a) και f(b) μετράνε μία φορά
    double total = f(a) + f(b);
    for (int i = 0; i < num_threads; i++)
        total += args[i].result;
    double result = (h / 2.0) * total;

    double elapsed = (te.tv_sec - ts.tv_sec) + (te.tv_nsec - ts.tv_nsec) / 1e9;

    printf("Integral from %.4f to %.4f with n=%d, threads=%d: %.10f\n", a, b, n, num_threads, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
