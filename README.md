# Assignment 2 — OpenMP Programming

**Course:** Selected Topics in Parallel and Distributed Computing  
**Submission:** C source files, `Makefile`, PDF report, and reproducibility materials  
**Platform used for experiments:** Apple MacBook Pro, Apple M4 Pro (12 CPU cores: 8 performance + 4 efficiency), 24 GB unified memory, macOS / Apple Silicon (ARM64)  
**C compiler:** Homebrew GNU GCC 16.2.0 (`gcc-16`)  
**Build flags:** `-O2 -fopenmp -std=c11 -Wall -Wextra`; link `-lm`

## Project contents

| File | Purpose |
|---|---|
| `matrix_mul.c` | Task 1 (Q1–Q5): serial and parallel multiplication; correctness; times; speedup; efficiency |
| `scheduling.c` | Task 2 (Q6–Q10): static, dynamic(1), dynamic(16), guided scheduling for regular and triangular workloads |
| `race.c` | Task 3 (Q11–Q16): intentionally racy counter and `critical`, `atomic`, `reduction` fixes |
| `false_sharing.c` | Task 4 (Q17–Q22): adjacent counters versus 64-byte-padded counters |
| `Makefile` | Build all four programs and optional targets to rerun individual experiments |
| `task5_plots.py` *(if included)* | Task 5 (Q23): optional plotting script that generates speedup and efficiency figures |
| `task5_speedup.png`, `task5_efficiency.png` | Task 5 figures to include in the report |
| `task*_results*.csv`, `task*_execution_log*.txt` | Measured experiment results and terminal records |
| `Assignment2_Report.pdf` *(name may differ)* | Written report covering Questions 1–26 |

> The `matrix_mul.c` you submit should be the **final controlled-rerun implementation**, renamed from `matrix_mul_rerun.c` as previously discussed. The earlier `matrix_mul.c` was a different implementation. Keep the final report's measured data consistent with the chosen final run.

## Requirements

- macOS with Homebrew GNU GCC and OpenMP support
- GNU Make (`make`, typically available with Xcode Command Line Tools)
- Optional: Python 3 + Matplotlib to recreate Task 5 figures

Verify tools in the VS Code integrated terminal:

```bash
gcc-16 --version
make --version
```

If GCC is missing, install it with `brew install gcc` and check the versioned command. The provided Makefile is configured for `gcc-16`. If your installed version differs, edit the `CC` setting or override it, for example `make CC=gcc-15`.

## Compile

From the directory containing `Makefile` and all four C source files:

```bash
cd ~/code/Dalya/Assignment2
make
```

This creates four executables: `matrix_mul`, `scheduling`, `race`, and `false_sharing`.

Build one program only:

```bash
make matrix_mul
make scheduling
make race
make false_sharing
```

To rebuild after editing a source file, run `make` again. To force a full rebuild, run `make clean` followed by `make`.

## Run experiments

Run **one target at a time** from the project directory:

```bash
make run-task1
make run-task2
make run-task3
make run-task4
```

The run targets display output and save a separate text execution log through `tee`. The C programs also export CSV results in the current directory. Re-running a task can **overwrite its generated CSV and log**; back up any final measured output before repeating an experiment.

| Task | Program | Output CSV | Execution log |
|---|---|---|---|
| 1 | `matrix_mul` | `task1_results_rerun.csv` (final controlled-rerun source) | `task1_execution_log_final.txt` |
| 2 | `scheduling` | `task2_results.csv` | `task2_execution_log.txt` |
| 3 | `race` | `task3_results.csv` | `task3_execution_log.txt` |
| 4 | `false_sharing` | `task4_results.csv` | `task4_execution_log.txt` |

**Task 1 filename note:** The controlled-rerun source currently writes `task1_results_rerun.csv`. If the submission uses the preferred final filename `task1_results.csv`, either rename the resulting CSV *after* running the program or update its `fopen` and status-message strings in the C source; make the filenames consistent. Do not replace an earlier CSV without first saving a backup if you need the history.

If a run seems to show no output, the process may still be computing or its standard output may be buffered through `tee`; it is complete when the command prompt returns.

## Experiment setup and validation

**Task 1: Parallel matrix multiplication**
- `N=2048`, `double` precision, 5 timed repetitions per configuration.
- Serial baseline and OpenMP tests with 1, 2, 4, 8, 16 requested threads.
- Includes an untimed warm-up, verification against serial output, and a check of actual OpenMP team sizes.
- Speedup: `mean_serial_time / mean_parallel_time`; efficiency: `speedup / requested_threads * 100%`.
- The machine has 12 CPU cores, but OpenMP can create 16 **software** threads. This does not imply 16 physical cores.

**Task 2: Scheduling**
- 8 threads; full square matrix multiplication and a triangular workload whose per-row work grows with the row index.
- Compares static, dynamic(1), dynamic(16), and guided scheduling; 5 measurements per schedule/workload.
- The triangular case computes a different amount of work than the full square case; compare scheduling strategies **within** each workload.

**Task 3: Race condition**
- 10,000,000 increments, 8 threads, 5 runs for each method.
- The unsynchronized case deliberately contains a data race and may produce invalid or varying results; `volatile` is *not* synchronization.
- The correct versions use `critical`, `atomic`, and `reduction`. The very low reduction time may be influenced by compiler optimization of the simple loop.

**Task 4: False sharing**
- 10,000,000 increments per thread, with 4 and 8 threads; 5 runs per configuration.
- Compares adjacent counters with 64-byte-aligned padded counters.
- Padding speedup: `mean_unpadded_time / mean_padded_time`.
- 64-byte slots meet the assignment specification; the observed benefit depends on hardware/coherence behavior.

**Task 5: Scaling analysis**
- Uses Task 1's **final** measured runtime data; no additional C benchmark is required.
- Figures: measured speedup against ideal linear speedup; parallel efficiency against number of threads.
- The report discusses departures from linear scaling, at least three potential bottlenecks, and two proposed optimizations.
- If you have `task5_plots.py`, run `python3 task5_plots.py` after installing Matplotlib. Otherwise, include the already generated figures and the underlying measured data.

## Final Task 1 reference results (chosen final run)

Mean serial baseline: **21.197242 seconds**.

| Threads | Mean runtime (s) | Speedup | Efficiency |
|---:|---:|---:|---:|
| 1 | 21.457594 | 0.9879 | 98.79% |
| 2 | 10.531808 | 2.0127 | 100.63% |
| 4 | 6.910633 | 3.0673 | 76.68% |
| 8 | 3.293501 | 6.4361 | 80.45% |
| 16 | 3.323797 | 6.3774 | 39.86% |

These are measured values from the selected final execution, **not predictions**. Earlier sessions had a variable serial baseline; keep the PDF figures and tables tied to this one consistent dataset.

## Submission checklist

- [ ] Four source files: `matrix_mul.c`, `scheduling.c`, `race.c`, `false_sharing.c`
- [ ] One `Makefile` and this `README.md`
- [ ] PDF report containing all required questions, tables, formulas, and Task 5 figures
- [ ] Final experimental CSV files and execution logs (recommended reproducibility evidence)
- [ ] Optional Task 5 plotting Python script and generated PNG figures
- [ ] Verify `make` succeeds on the submitted source files

To remove compiled executables **without deleting results**:

```bash
make clean
```
