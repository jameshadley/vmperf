/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "vm_perf_disk.h"
#include "dep/seeker.h"

static const char * disk_io_types[DISK_NUM_IO_TYPES] = {"random", "sequential", "cached_sequential"};
static const int    disk_io_size[DISK_NUM_IO_TYPES]  = {4096, 	   256*1024,	 256*1024};
static const char * disk_io_unit[DISK_NUM_IO_TYPES]  = {"KB", "MB", "MB"};
static const int 	disk_io_divisor[DISK_NUM_IO_TYPES]  = {1024, 1024*1024, 1024*1024};

static float test_disk_write(const int size, const unsigned int buf_size, const int flags, const int random){
	struct timeval tvStart, tvEnd;
	char *home, filename[PATH_MAX];
	void * buf;
	int fd;

	if((home = getenv("HOME")) == NULL){
		return 1;
	}

	snprintf(filename, PATH_MAX, "%s/vm_perf.temp", home);

	if((fd = open(filename, O_WRONLY|O_CREAT|flags, 0666)) == -1){
		perror("open");
		return 0;
	}

	if(posix_memalign(&buf, 4096, buf_size) != 0){ // Needed only by O_DIRECT
		return 0;
	}
	memset(buf, 'x', buf_size);

	gettimeofday(&tvStart, NULL);
	int left = size;
	while(left > 0){
		if(random){
			off64_t offset = (off64_t) rand() % ((size/buf_size)-1);
			if(lseek64(fd, buf_size * offset, SEEK_SET) == (off64_t) -1){
				perror("lseek64");
				break;
			}
		}

		int bytes = write(fd, buf, buf_size);
		if(bytes == -1){
			perror("write");
			break;
		}
		left -= bytes;
	}
	gettimeofday(&tvEnd, NULL);

	close(fd);
	free(buf);
	unlink(filename);

	float time_s = (float)(((tvEnd.tv_sec*1000000) + tvEnd.tv_usec) - ((tvStart.tv_sec*1000000) + tvStart.tv_usec))/1000000.0f;
	float mb_s = (size/time_s)/(1024.0f*1024.0f);

	return mb_s;
}


static int enumerate_disks(struct disk_result *r){
	FILE * fin = fopen("/proc/mounts", "r");
	if(fin == NULL){
		perror("fopen");
		return 1;
	}
	size_t line_size = 1000;
	char * line = malloc(line_size*sizeof(char));
	if(line == NULL){
		perror("malloc");
		return 1;
	}

	while(getline(&line, &line_size, fin) > 0){
		if(strncmp(line, "/dev/", 5) == 0){
			if(strncmp(line, "/dev/xvd", 8) == 0){
				line[9] = '\0';
			} else if(strncmp(line, "/dev/loop", 9) == 0){
				continue;
			} else {
				line[8] = '\0';
			}
			
			int found = 0;
			int i;
			for(i=0; i < r->num_disks; i++){
				if(strcmp(r->disk_stats[i].devname, line) == 0){
					found = 1;
					break;
				}
			}
			if(found == 0){
				++r->num_disks;
				r->disk_stats = realloc(r->disk_stats, sizeof(struct disk_stat)*r->num_disks);
				strncpy(r->disk_stats[r->num_disks-1].devname, line, 10);
			}
		}
	}
	fclose(fin);
	free(line);
	return 0;
};

void disk_bench(struct disk_result *r){
	bzero(r, sizeof(struct disk_result));

	if(enumerate_disks(r) == 1)
		return;

	int i;
	for(i=0; i < r->num_disks; ++i){
		//seeker(&r->disk_stats[i], r->disk_stats[i].devname, disk_io_size);	// Seeks, random access time
	}

	r->write[0] = test_disk_write(100*1024*1024, 256*1024, O_DIRECT, 1);
	r->write[1] = test_disk_write(100*1024*1024, 256*1024, O_DIRECT, 0);	// Direct write to disk
	r->write[2] = test_disk_write(100*1024*1024, 256*1024, 0, 0);			// Cached write to disk
};

void disk_report(const struct disk_result *r){
	int t;
	char delim = ' ';
	printf("\"storage\":{");
		printf("\"write_test\":[");
	for(t=0; t < DISK_NUM_IO_TYPES; t++){
		printf("%c{\"type\":\"%s\",\"rate\":\"%.2fMB/s\",\"buf_size\":\"%ib\"}",
			delim, disk_io_types[t], r->write[t], 256*1024);
		delim = ',';
	}
	printf("],");


	printf("\"disks\":[");

	int i;
	char disk_delim = ' ';
	for(i=0; i < r->num_disks; ++i){
		printf("%c{\"name\":\"%s\",", disk_delim, r->disk_stats[i].devname);
		printf("\"size\":\"%.2fGB\",", (float)(r->disk_stats[i].num_blocks*r->disk_stats[i].block_size)/(1024*1024*1024));
		printf("\"blocks\":\"%lu\",", r->disk_stats[i].num_blocks);
		printf("\"block_size\":\"%lub\"", r->disk_stats[i].block_size);

		printf(",\"read_tests\":[");

		delim = ' ';
		for(t=0; t < DISK_NUM_IO_TYPES; t++){
			printf("%c{\"type\":\"%s\",", delim, disk_io_types[t]);
			printf("\"buf_size\":\"%ib\",", disk_io_size[t]);
			printf("\"seek/read/s\":\"%i\",", 		r->disk_stats[i].seeks[t]);
			printf("\"access_time\":\"%.2fms\",",	r->disk_stats[i].access_time[t]);
			printf("\"rate\":\"%.2f%s/s\"", 		(float)(r->disk_stats[i].seeks[t]*r->disk_stats[i].block_size)/disk_io_divisor[t], disk_io_unit[t]);
			printf("}");
			delim = ',';
		}
		printf("]}");
		disk_delim = ',';
	}
	printf("]}");
};
