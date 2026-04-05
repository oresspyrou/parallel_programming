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

/** @brief Επόμενος κόμβος προς επεξεργασία — τα threads τον διαβάζουν/αυξάνουν δυναμικά. */
int next_i = 1;

/**
 * @brief Ορίσματα που περνάει το main σε κάθε thread.
 */
struct ThreadArgs {
    double a;        // κάτω όριο (για να υπολογίζει a + i*h)
    double h;        // βήμα — υπολογισμένο στο main
    int    n;           // συνολικός αριθμός τραπεζίων — άνω όριο του loop
    int chunk_size;    // πόσους κόμβους επεξεργάζεται κάθε φορά (για δυναμική κατανομή)
};


/**
 * @brief Συνάρτηση που εκτελεί κάθε thread — dynamic scheduling.
 *
 * Επαναλαμβάνει: παίρνει δυναμικά το επόμενο chunk από την next_i,
 * υπολογίζει το τοπικό άθροισμα, και το προσθέτει στην global_sum.
 * Σταματά όταν δεν υπάρχουν άλλοι κόμβοι προς επεξεργασία.
 *
 * @param arg  Δείκτης σε ThreadArgs
 * @return     NULL
 */
void* thread_func(void* arg) {
    ThreadArgs* t = (ThreadArgs*) arg;

    while (true) {
        // [LOCK] πάρε το επόμενο chunk και προχώρα τον μετρητή
        pthread_mutex_lock(&mutex);
        int i_start = next_i;
        next_i += t->chunk_size;
        pthread_mutex_unlock(&mutex);

        // Δεν υπάρχει άλλη δουλειά
        if (i_start >= t->n) break;

        // Υπολόγισε το κομμάτι [i_start, i_end)
        int i_end = i_start + t->chunk_size;
        if (i_end > t->n) i_end = t->n;  // μην ξεπεράσεις το n

        double local_sum = 0.0;
        for (int i = i_start; i < i_end; i++) {
            local_sum += f(t->a + i * t->h);  // x_i = a + i*h
        }

        // [LOCK] πρόσθεσε στην global_sum
        pthread_mutex_lock(&mutex);
        global_sum += 2.0 * local_sum;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}


/**
 * @brief Κύριο πρόγραμμα. Διαβάζει a, b, n, num_threads, chunk_size από command line,
 *        μοιράζει δυναμικά τους κόμβους σε threads και εκτυπώνει αποτέλεσμα + χρόνο.
 */
int main(int argc, char* argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s a b n num_threads chunk_size\n", argv[0]);
        return 1;
    }

    double a           = atof(argv[1]);
    double b           = atof(argv[2]);
    int    n           = atoi(argv[3]);
    int    num_threads = atoi(argv[4]);
    int    chunk_size  = atoi(argv[5]);

    // Validate input
    if (n <= 0 || num_threads <= 0 || chunk_size <= 0) {
        fprintf(stderr, "Error: n, num_threads and chunk_size must be positive\n");
        return 1;
    }

    double h = (b - a) / n;  // width of each trapezoid

    ThreadArgs args[num_threads];
    pthread_t  threads[num_threads];

    // Όλα τα threads μοιράζονται τα ίδια args — η κατανομή γίνεται δυναμικά μέσω next_i
    for (int i = 0; i < num_threads; i++) {
        args[i].a          = a;
        args[i].h          = h;
        args[i].n          = n;
        args[i].chunk_size = chunk_size;
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

    printf("Integral from %.4f to %.4f with n=%d, threads=%d, chunk=%d: %.10f\n", a, b, n, num_threads, chunk_size, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}