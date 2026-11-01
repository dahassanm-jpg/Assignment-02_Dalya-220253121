/* Dalya Awad Mohammad Hassan 220253121 - Assignment 2 - Task 1: controlled OpenMP matrix multiplication rerun.
   Compile (Homebrew GCC): gcc-15 -O2 -fopenmp matrix_mul_rerun.c -o matrix_mul_rerun -lm
   Substitute your installed version of GCC if needed. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#ifndef N
#define N 2048
#endif
#ifndef REPEATS
#define REPEATS 5
#endif
#define CONFIGS 5
#define TOLERANCE 1e-9

static void matmul_serial(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += A[(size_t)i*n+k] * B[(size_t)k*n+j];
            C[(size_t)i*n+j] = sum;
        }
    }
}

static void matmul_parallel(const double *A, const double *B,
                            double *C, int n, int threads) {
    #pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += A[(size_t)i*n+k] * B[(size_t)k*n+j];
            C[(size_t)i*n+j] = sum;
        }
    }
}

static double max_absolute_error(const double *ref, const double *result,
                                 size_t entries) {
    double max_err = 0.0;
    for (size_t i = 0; i < entries; ++i) {
        if (!isfinite(ref[i]) || !isfinite(result[i])) return INFINITY;
        double err = fabs(ref[i] - result[i]);
        if (err > max_err) max_err = err;
    }
    return max_err;
}

/* Checks actual team size; called outside timed multiplication. */
static int actual_team_size(int requested) {
    int actual = 0;
    #pragma omp parallel num_threads(requested) shared(actual)
    {
        #pragma omp single
        actual = omp_get_num_threads();
    }
    return actual;
}

int main(void) {
    const int requested[CONFIGS] = {1, 2, 4, 8, 16};
    int actual[CONFIGS];
    double means[CONFIGS], speedups[CONFIGS], efficiencies[CONFIGS];
    double errors[CONFIGS];
    size_t entries = (size_t)N * N;
    size_t bytes = entries * sizeof(double);
    double *A = malloc(bytes), *B = malloc(bytes);
    double *C_serial = malloc(bytes), *C_parallel = malloc(bytes);
    if (!A || !B || !C_serial || !C_parallel) {
        fprintf(stderr, "Matrix allocation failed.\n");
        free(A); free(B); free(C_serial); free(C_parallel);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            A[(size_t)i*N+j] = (double)((i+j)%100)/100.0;
            B[(size_t)i*N+j] = (double)((i*3+j)%100)/100.0;
        }

    omp_set_dynamic(0);
    printf("Assignment 2 Task 1 - controlled rerun\n");
    printf("N=%d, repeats=%d, available processors=%d, default max threads=%d\n",
           N, REPEATS, omp_get_num_procs(), omp_get_max_threads());

    /* Warm up serial code before timing. */
    matmul_serial(A, B, C_serial, N);
    double serial_total = 0.0;
    printf("\nSERIAL BASELINE\n");
    for (int r = 0; r < REPEATS; ++r) {
        double t0 = omp_get_wtime();
        matmul_serial(A, B, C_serial, N);
        double elapsed = omp_get_wtime() - t0;
        serial_total += elapsed;
        printf("Serial run %d: %.6f s\n", r+1, elapsed);
    }
    double baseline = serial_total / REPEATS;
    printf("Serial mean: %.6f s\n", baseline);

    int pass = 1;
    for (int c = 0; c < CONFIGS; ++c) {
        int p = requested[c];
        actual[c] = actual_team_size(p); /* outside timed region */
        printf("\nRequested threads: %d | Actual threads: %d\n", p, actual[c]);
        if (actual[c] != p) {
            printf("WARNING: OpenMP did not use the requested thread count.\n");
        }
        matmul_parallel(A, B, C_parallel, N, p); /* untimed warm-up */
        double total = 0.0, max_err = 0.0;
        for (int r = 0; r < REPEATS; ++r) {
            double t0 = omp_get_wtime();
            matmul_parallel(A, B, C_parallel, N, p);
            double elapsed = omp_get_wtime() - t0;
            double err = max_absolute_error(C_serial, C_parallel, entries);
            if (err > max_err) max_err = err;
            if (!isfinite(err) || err > TOLERANCE) pass = 0;
            total += elapsed;
            printf("Run %d: %.6f s | Max error: %.12g\n", r+1, elapsed, err);
        }
        means[c] = total / REPEATS;
        speedups[c] = baseline / means[c];
        efficiencies[c] = 100.0 * speedups[c] / p;
        errors[c] = max_err;
        printf("Mean: %.6f s | Speedup: %.6f | Efficiency: %.2f%% | %s\n",
               means[c], speedups[c], efficiencies[c],
               (isfinite(max_err) && max_err <= TOLERANCE) ? "PASS" : "FAIL");
    }

    puts("\nSUMMARY");
    puts("Requested  Actual   Mean (s)    Speedup   Efficiency (%)   Max error");
    for (int c = 0; c < CONFIGS; ++c)
        printf("%-10d %-8d %-11.6f %-9.4f %-16.2f %.12g\n",
               requested[c], actual[c], means[c], speedups[c],
               efficiencies[c], errors[c]);
    printf("Serial baseline: %.6f s | Overall correctness: %s\n",
           baseline, pass ? "PASS" : "FAIL");

    FILE *f = fopen("task1_results_rerun.csv", "w");
    if (f) {
        fprintf(f, "requested_threads,actual_threads,mean_time_seconds,speedup,efficiency_percent,max_absolute_error\n");
        for (int c = 0; c < CONFIGS; ++c)
            fprintf(f, "%d,%d,%.9f,%.9f,%.9f,%.12g\n",
                    requested[c], actual[c], means[c], speedups[c],
                    efficiencies[c], errors[c]);
        fclose(f);
        puts("Saved: task1_results_rerun.csv");
    } else {
        fprintf(stderr, "WARNING: Could not save the CSV.\n");
    }
    free(A); free(B); free(C_serial); free(C_parallel);
    return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
