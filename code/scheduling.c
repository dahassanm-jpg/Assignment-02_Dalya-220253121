/* Dalya Awad Mohammad Hassan 220253121 - Assignment 2 - Task 2: OpenMP Scheduling Comparison.
 * Compile: gcc-16 -O2 -fopenmp scheduling.c -o scheduling -lm
 * Execute: ./scheduling | tee task2_execution_log.txt
 */
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
#define THREADS 8
#define SCHEMES 4

typedef struct { const char *name; omp_sched_t kind; int chunk; } Scheme;
static const Scheme schemes[] = {
    {"static", omp_sched_static, 0},
    {"dynamic_1", omp_sched_dynamic, 1},
    {"dynamic_16", omp_sched_dynamic, 16},
    {"guided", omp_sched_guided, 1}
};

static void multiply(const double *A, const double *B, double *C,
                     int n, int triangular) {
    #pragma omp parallel for num_threads(THREADS) schedule(runtime)
    for (int i = 0; i < n; ++i) {
        int limit = triangular ? (i + 1) : n;
        for (int j = 0; j < limit; ++j) {
            double sum = 0.0;
            for (int k = 0; k < n; ++k)
                sum += A[(size_t)i*n+k] * B[(size_t)k*n+j];
            C[(size_t)i*n+j] = sum;
        }
    }
}

static double max_error(const double *reference, const double *result,
                        int n, int triangular) {
    double maximum = 0.0;
    for (int i = 0; i < n; ++i) {
        int limit = triangular ? i + 1 : n;
        for (int j = 0; j < limit; ++j) {
            size_t index = (size_t)i*n+j;
            if (!isfinite(reference[index]) || !isfinite(result[index]))
                return INFINITY;
            double error = fabs(reference[index] - result[index]);
            if (error > maximum) maximum = error;
        }
    }
    return maximum;
}

int main(void) {
    const size_t cells = (size_t)N * N;
    double *A = malloc(cells*sizeof(double));
    double *B = malloc(cells*sizeof(double));
    double *C = calloc(cells,sizeof(double));
    double *reference = calloc(cells,sizeof(double));
    if (!A || !B || !C || !reference) {
        fprintf(stderr,"Memory allocation failed\n");
        free(A);free(B);free(C);free(reference);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            A[(size_t)i*N+j] = (double)((i+j)%100)/100.0;
            B[(size_t)i*N+j] = (double)((i*3+j)%100)/100.0;
        }
    omp_set_dynamic(0);
    FILE *csv = fopen("task2_results.csv","w");
    if (!csv) { perror("CSV");free(A);free(B);free(C);free(reference);return EXIT_FAILURE; }
    fprintf(csv,"workload,schedule,run1_s,run2_s,run3_s,run4_s,run5_s,mean_s,max_absolute_error\n");
    printf("Task 2 | N=%d | threads=%d | repetitions=%d | available processors=%d\n",
           N,THREADS,REPEATS,omp_get_num_procs());
    int failed = 0;
    for (int triangular = 0; triangular <= 1; ++triangular) {
        const char *workload = triangular ? "triangular" : "full";
        printf("\nWORKLOAD: %s\n",workload);
        omp_set_schedule(omp_sched_static,0);
        multiply(A,B,reference,N,triangular); /* untimed reference */
        for (int s = 0; s < SCHEMES; ++s) {
            omp_set_schedule(schemes[s].kind,schemes[s].chunk);
            multiply(A,B,C,N,triangular); /* untimed warm-up */
            double times[REPEATS], total = 0.0, maximum = 0.0;
            for (int r = 0; r < REPEATS; ++r) {
                const double t0 = omp_get_wtime();
                multiply(A,B,C,N,triangular);
                times[r] = omp_get_wtime()-t0;
                total += times[r];
                const double error = max_error(reference,C,N,triangular);
                if (error > maximum) maximum = error;
                if (!isfinite(error) || error > 1e-9) failed = 1;
            }
            const double mean = total/REPEATS;
            printf("%-11s mean=%.6f s | runs=",schemes[s].name,mean);
            for (int r = 0; r < REPEATS; ++r)
                printf("%s%.6f",r?",":"",times[r]);
            printf(" | max error=%.12g\n",maximum);
            fprintf(csv,"%s,%s",workload,schemes[s].name);
            for (int r = 0; r < REPEATS; ++r) fprintf(csv,",%.9f",times[r]);
            fprintf(csv,",%.9f,%.12g\n",mean,maximum);
            fflush(csv);
        }
    }
    fclose(csv);
    printf("\nOverall correctness: %s\nSaved task2_results.csv\n",failed?"FAIL":"PASS");
    free(A);free(B);free(C);free(reference);
    return failed?EXIT_FAILURE:EXIT_SUCCESS;
}
