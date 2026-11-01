# Dalya Awad Mohammad Hassan 220253121 - Assignment 2 - OpenMP Programming
# macOS / Homebrew GCC 16

CC = gcc-16
CFLAGS = -O2 -fopenmp -std=c11 -Wall -Wextra
LDLIBS = -lm
PROGRAMS = matrix_mul scheduling race false_sharing

.PHONY: all clean run-task1 run-task2 run-task3 run-task4

all: $(PROGRAMS)

matrix_mul: matrix_mul.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

scheduling: scheduling.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

race: race.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

false_sharing: false_sharing.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

run-task1: matrix_mul
	./matrix_mul 2>&1 | tee task1_execution_log.txt

run-task2: scheduling
	./scheduling 2>&1 | tee task2_execution_log.txt

run-task3: race
	./race 2>&1 | tee task3_execution_log.txt

run-task4: false_sharing
	./false_sharing 2>&1 | tee task4_execution_log.txt

clean:
	rm -f $(PROGRAMS)
