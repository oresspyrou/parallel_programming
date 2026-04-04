//Headers
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/**
 * @brief Η συνάρτηση προς ολοκλήρωση.
 * @param x  Τιμή της μεταβλητής
 * @return   sin(x)
 */
double f(double x) {
    return sin(x);
}

/**
 * @brief Υπολογίζει το ολοκλήρωμα της f(x) με τον τραπεζοειδή κανόνα.
 *
 * Χωρίζει το [a,b] σε n ίσα τμήματα και αθροίζει τα εμβαδά των τραπεζίων.
 *
 * @param a  Αρχή διαστήματος
 * @param b  Τέλος διαστήματος
 * @param n  Αριθμός τραπεζίων
 * @return   Προσεγγιστική τιμή του ολοκληρώματος
 */
double trapezoidal(double a, double b, int n) {
    double h   = (b - a) / n;
    double sum = f(a) + f(b);        
    
    for (int i = 1; i < n; i++) {
        sum += 2.0 * f(a + i * h);   
    }
    
    return (h / 2.0) * sum;
}


/**
 * @brief Κύριο πρόγραμμα. Διαβάζει a, b, n από command line και εκτυπώνει
 *        το αποτέλεσμα του ολοκληρώματος μαζί με τον χρόνο εκτέλεσης.
 */
int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s a b n\n", argv[0]);
        return 1;
    }

    double a = atof(argv[1]);
    double b = atof(argv[2]);
    int    n = atoi(argv[3]);

    struct timespec ts, te;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    double result = trapezoidal(a, b, n);

    clock_gettime(CLOCK_MONOTONIC, &te);
    double elapsed = (te.tv_sec - ts.tv_sec) + (te.tv_nsec - ts.tv_nsec) / 1e9;

    printf("Integral from %.4f to %.4f with n=%d: %.10f\n", a, b, n, result);
    printf("Time: %.6f seconds\n", elapsed);

    return 0;
}