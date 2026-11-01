
"""
Dalya Awad Mohammad Hassan 220253121 - Assignment 2 - Task 5
Q23: Speedup and Efficiency Plots

Source: Final Task 1 experiment
Hardware: Apple M4 Pro
Compiler: GCC 16.2.0
"""

import matplotlib.pyplot as plt
from pathlib import Path

# Final measured data from Task 1
threads = [1, 2, 4, 8, 16]

serial_time = 21.197242

parallel_times = [
    21.457594,
    10.531808,
    6.910633,
    3.293501,
    3.323797
]

# Calculate speedup and efficiency
speedup = [
    serial_time / t for t in parallel_times
]

efficiency = [
    (s / p) * 100
    for s, p in zip(speedup, threads)
]

ideal_speedup = threads
ideal_efficiency = [100] * len(threads)

output_dir = Path(__file__).resolve().parent

# ---------------------------------------
# Figure 1: Speedup vs. Threads
# ---------------------------------------

plt.figure(figsize=(8, 5))

plt.plot(
    threads,
    speedup,
    marker="o",
    linewidth=2,
    label="Measured Speedup"
)

plt.plot(
    threads,
    ideal_speedup,
    marker="s",
    linestyle="--",
    label="Ideal Linear Speedup"
)

plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Task 5: Speedup vs. Threads")

plt.xticks(threads)
plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()

plt.savefig(
    output_dir / "task5_speedup.png",
    dpi=300
)

plt.close()

# ---------------------------------------
# Figure 2: Efficiency vs. Threads
# ---------------------------------------

plt.figure(figsize=(8, 5))

plt.plot(
    threads,
    efficiency,
    marker="o",
    linewidth=2,
    label="Measured Efficiency"
)

plt.axhline(
    y=100,
    linestyle="--",
    label="Ideal Efficiency (100%)"
)

plt.xlabel("Number of Threads")
plt.ylabel("Efficiency (%)")
plt.title("Task 5: Efficiency vs. Threads")

plt.xticks(threads)
plt.ylim(0, 115)
plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()

plt.savefig(
    output_dir / "task5_efficiency.png",
    dpi=300
)

plt.close()

# ---------------------------------------
# Print calculated results
# ---------------------------------------

print("\nTask 5: Scaling Results")
print("-" * 56)

print(
    f"{'Threads':<10}"
    f"{'Time (s)':<14}"
    f"{'Speedup':<14}"
    f"{'Efficiency (%)':<14}"
)

for p, t, s, e in zip(
    threads, parallel_times, speedup, efficiency
):
    print(
        f"{p:<10}"
        f"{t:<14.6f}"
        f"{s:<14.4f}"
        f"{e:<14.2f}"
    )

print("\nFigures saved:")
print("task5_speedup.png")
print("task5_efficiency.png")
