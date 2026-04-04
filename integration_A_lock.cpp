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

/** @brief Mutex που προστατεύει την global_sum από race conditions. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/** @brief Κοινή μεταβλητή όπου τα threads προσθέτουν τα μερικά αθροίσματά τους. */
double global_sum = 0.0;

/**
 * @brief Ορίσματα που περνάει το main σε κάθε thread.
 */
struct ThreadArgs {
    int    start;    // πρώτο i που επεξεργάζεται
    int    end;      // τελευταίο i (exclusive)
    double a;        // κάτω όριο (για να υπολογίζει a + i*h)
    double h;        // βήμα — υπολογισμένο στο main
};


/**
 * @brief Συνάρτηση που εκτελεί κάθε thread.
 *
 * Υπολογίζει τοπικά το μερικό άθροισμα για τους κόμβους [start, end),
 * και στη συνέχεια το προσθέτει στην global_sum με mutex lock.
 *
 * @param arg  Δείκτης σε ThreadArgs
 * @return     NULL
 */
void* thread_func(void* arg) {
    ThreadArgs* t = (ThreadArgs*) arg;

    double local_sum = 0.0;
    for (int i = t->start; i < t->end; i++) {
        local_sum += f(t->a + i * t->h);  // x_i = a + i*h
    }

    pthread_mutex_lock(&mutex);
    global_sum += 2.0 * local_sum;
    pthread_mutex_unlock(&mutex);

    return NULL;
}


/**
 * @brief Κύριο πρόγραμμα. Διαβάζει a, b, n, num_threads από command line,
 *        μοιράζει τον υπολογισμό σε threads με mutex-protected global_sum,
 *        και εκτυπώνει αποτέλεσμα + χρόνο.
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

    double h = (b - a) / n;  // width of each trapezoid

    ThreadArgs args[num_threads];
    pthread_t  threads[num_threads];

    // Τα threads καλύπτουν μόνο i = 1 έως n-1 (τα άκρα f(a)+f(b) υπολογίζονται στο main)
    int chunk = (n - 1) / num_threads;
    for (int i = 0; i < num_threads; i++) {
        args[i].start  = 1 + i * chunk;
        args[i].end    = (i == num_threads - 1) ? n : 1 + (i + 1) * chunk;
        args[i].a      = a;
        args[i].h      = h;
    }

    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // Launch all threads
    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, thread_func, &args[i]);

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &te);

    // Combine endpoints (counted once) with the shared sum from all threads
    double total  = f(a) + f(b) + global_sum;
    double result = (h / 2.0) * total;

    double elapsed = (te.tv_sec - ts.tv_sec) + (te.tv_nsec - ts.tv_nsec) / 1e9;

    printf("Integral from %.4f to %.4f with n=%d, threads=%d: %.10f\n", a, b, n, num_threads, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}
