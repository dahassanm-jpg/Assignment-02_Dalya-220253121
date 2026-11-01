/*
 * Dalya Awad Mohammad Hassan 220253121 - Assignment 2 - Task 4: False Sharing (Questions 17-22)
 * Build (macOS Homebrew GCC):
 *   gcc-16 -O2 -fopenmp -std=c11 -Wall -Wextra false_sharing.c -o false_sharing
 * Run and log:
 *   ./false_sharing 2>&1 | tee task4_execution_log.txt
 *
 * Each of T threads increments only its own counter ITERATIONS times.
 * Volatile prevents optimization from eliminating the repeated stores.
 * This is a benchmarking aid, NOT synchronization.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <omp.h>

#ifndef ITERATIONS
#define ITERATIONS 10000000
#endif
#ifndef REPEATS
#define REPEATS 5
#endif

#define NUM_THREADS_CASES 2
#define CACHE_LINE 64

/* One counter per 64-byte slot, aligned to a 64-byte boundary. */
typedef struct {
    _Alignas(CACHE_LINE) volatile int counter;
    unsigned char padding[CACHE_LINE - sizeof(int)];
} PaddedCounter;

_Static_assert(sizeof(PaddedCounter) == CACHE_LINE,
               "Each padded counter must occupy exactly 64 bytes");
_Static_assert(_Alignof(PaddedCounter) >= CACHE_LINE,
               "Each padded counter must be 64-byte aligned");

static void unpadded_test(int threads, double *seconds, int *correct) {
    volatile int *counters = (volatile int *)calloc((size_t)threads, sizeof(int));
    if (!counters) { perror("calloc"); exit(EXIT_FAILURE); }

    double t0 = omp_get_wtime();
    #pragma omp parallel num_threads(threads) shared(counters)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ITERATIONS; ++i)
            counters[tid]++;
    }
    *seconds = omp_get_wtime() - t0;

    *correct = 1;
    for (int t = 0; t < threads; ++t)
        if (counters[t] != ITERATIONS) *correct = 0;
    free((void *)counters);
}

static void padded_test(int threads, double *seconds, int *correct) {
    /* aligned_alloc requires allocation size to be a multiple of alignment. */
    PaddedCounter *counters = aligned_alloc(CACHE_LINE, (size_t)threads * sizeof(PaddedCounter));
    if (!counters) { perror("aligned_alloc"); exit(EXIT_FAILURE); }
    for (int t = 0; t < threads; ++t) counters[t].counter = 0;

    double t0 = omp_get_wtime();
    #pragma omp parallel num_threads(threads) shared(counters)
    {
        int tid = omp_get_thread_num();
        for (int i = 0; i < ITERATIONS; ++i)
            counters[tid].counter++;
    }
    *seconds = omp_get_wtime() - t0;

    *correct = 1;
    for (int t = 0; t < threads; ++t)
        if (counters[t].counter != ITERATIONS) *correct = 0;
    free(counters);
}

int main(void) {
    const int cases[NUM_THREADS_CASES] = {4, 8};
    int failed = 0;
    omp_set_dynamic(0);
    setvbuf(stdout, NULL, _IONBF, 0);

    FILE *csv = fopen("task4_results.csv", "w");
    if (!csv) { perror("task4_results.csv"); return EXIT_FAILURE; }
    fprintf(csv, "threads,unpadded_mean_s,padded_mean_s,padding_speedup,unpadded_correct,padded_correct\n");

    printf("Task 4: False Sharing | increments per thread=%d | repeats=%d\n",
           ITERATIONS, REPEATS);
    printf("Available OpenMP processors: %d | padded slot: %zu bytes\n",
           omp_get_num_procs(), sizeof(PaddedCounter));

    for (int c = 0; c < NUM_THREADS_CASES; ++c) {
        const int threads = cases[c];
        double unpadded[REPEATS], padded[REPEATS];
        double total_u = 0.0, total_p = 0.0;
        int ok_u = 1, ok_p = 1;
        printf("\nTHREADS: %d\n", threads);

        /* Untimed warm-up for both layouts. */
        double warmup;
        int warmup_correct;
        unpadded_test(threads, &warmup, &warmup_correct);
        if (!warmup_correct) failed = 1;
        padded_test(threads, &warmup, &warmup_correct);
        if (!warmup_correct) failed = 1;

        /* Alternate which layout goes first to reduce order bias. */
        for (int r = 0; r < REPEATS; ++r) {
            int a, b;
            if (r % 2 == 0) {
                unpadded_test(threads, &unpadded[r], &a);
                padded_test(threads, &padded[r], &b);
            } else {
                padded_test(threads, &padded[r], &b);
                unpadded_test(threads, &unpadded[r], &a);
            }
            if (!a) ok_u = 0;
            if (!b) ok_p = 0;
            total_u += unpadded[r];
            total_p += padded[r];
            printf("Run %d: unpadded=%.6f s | padded=%.6f s | correctness=%s\n",
                   r+1, unpadded[r], padded[r], (a && b) ? "PASS" : "FAIL");
        }

        const double mean_u = total_u / REPEATS;
        const double mean_p = total_p / REPEATS;
        const double speedup = mean_u / mean_p;
        if (!ok_u || !ok_p) failed = 1;
        printf("MEAN: unpadded=%.6f s | padded=%.6f s | padding speedup=%.4fx | %s\n",
               mean_u, mean_p, speedup, (ok_u && ok_p) ? "PASS" : "FAIL");
        fprintf(csv, "%d,%.9f,%.9f,%.9f,%d,%d\n",
                threads, mean_u, mean_p, speedup, ok_u, ok_p);
        fflush(csv);
    }
    fclose(csv);
    printf("\nOverall correctness: %s\nSaved: task4_results.csv\n", failed ? "FAIL" : "PASS");
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
