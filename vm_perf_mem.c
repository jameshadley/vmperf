/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#include <strings.h>
#include <stdio.h>

#include "vm_perf_mem.h"
#include "dep/stream.h"

static const char * mem_test_labels[NUM_MEM_TESTS] = {
	"Copy", "Scale", "Add", "Triad" };

void mem_bench(struct mem_result* r){
	bzero(r, sizeof(struct mem_result)); // Clear result

	stream(r);
};

void mem_report(const struct mem_result* r){
	int i;
	printf("\"mem\":{");
	printf("\"stream\":[");
	char delim = ' ';
	for(i=0; i < NUM_MEM_TESTS; ++i){
		printf("%c{\"test\":\"%s\",\"result\":\"%.1fMB/s\"}", delim, mem_test_labels[i], r->rate[i]);
		delim = ',';
	}
	printf("]}");
};
