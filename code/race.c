/* Dalya Awad Mohammad Hassan 220253121 - Assignment 2 - Task 3: Race Condition and Its Fixes (Q11-Q16)
 * macOS: gcc-16 -O2 -fopenmp race.c -o race
 * Run: ./race 2>&1 | tee task3_execution_log.txt
 * A race is intentionally demonstrated in the first experiment.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#ifndef ITERATIONS
#define ITERATIONS 10000000
#endif
#ifndef REPEATS
#define REPEATS 5
#endif
#ifndef THREADS
#define THREADS 8
#endif

static void benchmark(const char *method, FILE *csv) {
    double total = 0.0;
    int all_correct = 1;
    printf("\n%s\n", method);
    for (int run = 1; run <= REPEATS; ++run) {
        /* Each method starts from 0; value from racy case is deliberately undefined. */
        int final_count = 0;
        double start, elapsed;
        if (strcmp(method, "race") == 0) {
            /* volatile prevents simple compiler elimination; NOT a race fix. */
            volatile int counter = 0;
            start = omp_get_wtime();
            #pragma omp parallel for num_threads(THREADS) shared(counter)
            for (int i = 0; i < ITERATIONS; ++i) {
                counter++;
            }
            elapsed = omp_get_wtime() - start;
            final_count = counter;
        } else if (strcmp(method, "critical") == 0) {
            int counter = 0;
            start = omp_get_wtime();
            #pragma omp parallel for num_threads(THREADS) shared(counter)
            for (int i = 0; i < ITERATIONS; ++i) {
                #pragma omp critical
                { counter++; }
            }
            elapsed = omp_get_wtime() - start;
            final_count = counter;
        } else if (strcmp(method, "atomic") == 0) {
            int counter = 0;
            start = omp_get_wtime();
            #pragma omp parallel for num_threads(THREADS) shared(counter)
            for (int i = 0; i < ITERATIONS; ++i) {
                #pragma omp atomic update
                counter++;
            }
            elapsed = omp_get_wtime() - start;
            final_count = counter;
        } else {
            int counter = 0;
            start = omp_get_wtime();
            #pragma omp parallel for num_threads(THREADS) reduction(+:counter)
            for (int i = 0; i < ITERATIONS; ++i) {
                counter++;
            }
            elapsed = omp_get_wtime() - start;
            final_count = counter;
        }
        total += elapsed;
        if (strcmp(method, "race") != 0 && final_count != ITERATIONS) all_correct = 0;
        printf("Run %d: counter=%d, time=%.6f s%s\n", run, final_count, elapsed,
               (strcmp(method, "race") == 0 ? " (race; result not guaranteed)" :
               (final_count == ITERATIONS ? " PASS" : " FAIL")));
        fprintf(csv, "%s,%d,%d,%d,%.9f\n", method, run, THREADS, final_count, elapsed);
        fflush(csv);
    }
    printf("Mean %s time: %.6f s", method, total / REPEATS);
    if (strcmp(method, "race") != 0) printf(" | %s", all_correct ? "PASS" : "FAIL");
    printf("\n");
}

int main(void) {
    omp_set_dynamic(0);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Task 3: race condition and fixes | iterations=%d | threads=%d | repetitions=%d\n",
           ITERATIONS, THREADS, REPEATS);
    printf("Available processors: %d\n", omp_get_num_procs());
    FILE *csv = fopen("task3_results.csv", "w");
    if (!csv) { perror("task3_results.csv"); return EXIT_FAILURE; }
    fprintf(csv, "method,run,threads,counter,time_seconds\n");
    benchmark("race", csv);
    benchmark("critical", csv);
    benchmark("atomic", csv);
    benchmark("reduction", csv);
    fclose(csv);
    printf("\nSaved: task3_results.csv\n");
    return EXIT_SUCCESS;
}
