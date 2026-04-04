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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
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