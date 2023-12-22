#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "vm_perf.h"


void vm_perf_report(const struct vm_perf_result *bm){
	printf("{\"vm_perf\":\"%s\",", VERSION);
	printf("\"modules\":{");



	sys_report(&bm->sys);	putchar(',');
	cpu_report(&bm->cpu);	putchar(',');
	net_report(&bm->net);	putchar(',');
	mem_report(&bm->mem);	putchar(',');
	disk_report(&bm->disk);

	printf("}}");
	fflush(stdout);
}

int main(const int argc, char * const argv[]){
	struct vm_perf_result benchmark;

	if(geteuid() != 0){
		fprintf(stderr, "Error: test must be run as root\n");
		return 1;
	}


	sys_info(&benchmark.sys);
	cpu_bench(&benchmark.cpu);
	net_bench(&benchmark.net);
	mem_bench(&benchmark.mem);
	disk_bench(&benchmark.disk);
	vm_perf_report(&benchmark);

	int i;
	for(i=0; i < NUM_INFO_PATHS; ++i)
		free(benchmark.sys.info[i]);

	free(benchmark.disk.disk_stats);

	return 0;
}
