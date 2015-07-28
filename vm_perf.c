#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "vm_perf.h"

/*
struct vm_perf_options{
	int time_limit;		// Maximum time to run tests
	int verbose;		// Output
};

static int parse_options(struct vm_perf_options *options, const int argc, char * const argv[]){
	int opt;
	while((opt = getopt(argc, argv, "hvt:")) != -1){
		switch(opt){
			case 'v': options->verbose = 1;				 break;
			case 't': options->time_limit = atoi(optarg); break;
			case 'h':
				printf("%s-%s\n", argv[0], VERSION);
				printf("Options:\n");
				printf("-v \t Enable verbose output\n");
				printf("-t 20 \t Time limit for testing in seconds\n");
				printf("-h help");
				break;
			default:
				fprintf(stderr, "Unknown option '%c'\n", opt);
				return 1;
		}
	}

	return 0;
};*/

void vm_perf_report(const struct vm_perf_result *bm){
	printf("{\"vm_perf\":\"%s\",", VERSION);
	printf("\"modules\":{");


	//printf("Memory:%s\n", bm->mem_info);
	//printf("Disk:%s\n", bm->disk_info);

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

	//if(parse_options(&options, argc, argv) == 1)
	//	return 1;

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
