/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#ifndef VM_PERF_CPU_H
#define VM_PERF_CPU_H

#define NUM_CPU_TESTS 3

struct cpu_result{
	unsigned char num_cores;	// Number of cores in processor
	unsigned char num_procs;	// Number of processors in system

	int cpu_timing[NUM_CPU_TESTS];			// Test CPU result
};

void cpu_bench(struct cpu_result * r);
void cpu_report(const struct cpu_result * r);

#endif
