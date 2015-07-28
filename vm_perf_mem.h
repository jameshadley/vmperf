/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#ifndef VM_PERF_MEM_H
#define VM_PERF_MEM_H

#define NUM_MEM_TESTS 4

struct mem_result{
	double rate[NUM_MEM_TESTS];	// Transfer rate in GB/s
};

void mem_bench(struct mem_result* r);
void mem_report(const struct mem_result* r);

#endif
