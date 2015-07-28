/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>

#include "vm_perf_sys.h"

static const char * info_path[NUM_INFO_PATHS] = {
	"/proc/version",
	"/proc/cpuinfo"
	//"/proc/meminfo",
	//"/proc/scsi/scsi",
};

static char* proc_get_info(const char * path){
	int fd = open(path, O_RDONLY);
	if(fd == -1){
		perror("open");
		return strdup("error");
	}

	size_t offset = 0;
	size_t size = 0;
	char * info = NULL;
	while(1){
		if(offset >= size){
			size += 100;
			info = (char*) realloc(info, sizeof(char)*size);
			if(info == NULL){
				perror("malloc");
				return strdup("error");
			}
		}

		int bytes = read(fd, &info[offset], size-offset);
		if(bytes < 0){
			perror("read");
			break;
		}else if(bytes == 0)
			break;

		offset+= bytes;
	}
	close(fd);
	info[offset-1] = '\0';

	return info;
}


static int enumerate_cpus(struct sys_result *r){
	FILE * fin = fopen("/proc/cpuinfo", "r");
	if(fin == NULL){
		perror("fopen");
		return 1;
	}
	size_t line_size = 100;
	char * line = malloc(line_size*sizeof(char));
	if(line == NULL){
		perror("malloc");
		return 1;
	}

	bzero(r->cpu_model, sizeof(r->cpu_model));
	r->cpu_count = 0;
	while(getline(&line, &line_size, fin) > 0){
		int len = strlen(line);
		line[len-1] = '\0';
		if(strncmp(line, "model name", 10) == 0){
			if(r->cpu_model[0] == 0){
				char * model = strchr(line, ':');
				if(++model){
					while(*model == ' '){
						model++;
					}
					strncpy(r->cpu_model, model, sizeof(r->cpu_model));
				}
			}
			r->cpu_count++;
		}
	}
	fclose(fin);
	free(line);
	return 0;
};

void sys_info(struct sys_result * r){
	bzero(r, sizeof(struct sys_result));

	int i;
	for(i=0; i < NUM_INFO_PATHS; ++i)
		r->info[i] = proc_get_info(info_path[i]);

	if(gethostname(r->hostname, HOST_NAME_MAX) == -1){
		perror("gethostname");
		return;
	}

	struct sysinfo mi;
	if(sysinfo(&mi) == -1){
		perror("sysinfo");
		return;
	}
	r->totalRAM = mi.totalram / (1024 * 1024);
	r->freeRAM  = mi.freeram  / (1024 * 1024);

	enumerate_cpus(r);
}

void sys_report(const struct sys_result * r){
	printf("\"system\":{");
		printf("\"uname\":\"%s\",", r->info[0]);
		printf("\"hostname\":\"%s\",", r->hostname);
		printf("\"cpu_model\":\"%s\",", r->cpu_model);
		printf("\"cpu_count\":\"%hu\",", r->cpu_count);
		printf("\"ram_total\":\"%luMB\",", r->totalRAM);
		printf("\"ram_free\":\"%luMB\"", r->freeRAM);
	printf("}");
}
