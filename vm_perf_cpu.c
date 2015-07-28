/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#include <omp.h>
#include "vm_perf_cpu.h"
#include "dep/c-ray.h"
#include "dep/dhry.h"

#include "vm_perf_cpu.h"

static const char * cpu_test_labels[NUM_CPU_TESTS] = {
	"DHRYSTONE",
	"C-RAY F",
	"C-RAY MT"
};

void cpu_bench(struct cpu_result * r){
	int num_cores = omp_get_num_procs();

	r->cpu_timing[0] = dhry(20);	// Run Dhrystone test for 20 seconds
	r->cpu_timing[1] = cray_f(1600, 900, 1);
	r->cpu_timing[2] = cray_mt(num_cores, 1600, 900, 1);
};

void cpu_report(const struct cpu_result * r){
	printf("\"cpu\":[");
	int i;
	char delim = ' ';
	for(i=0; i < NUM_CPU_TESTS; ++i){
		printf("%c{\"test\":\"%s\",\"result\":\"%i\"}", delim, cpu_test_labels[i], r->cpu_timing[i]);
		delim = ',';
	}
	printf("]");
};
