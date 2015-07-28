#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#include "seeker.h"

static int seeker_done = 0;
static time_t start, end;
void done(){
	time(&end);

	if (end < start + SEEKER_TIMEOUT) {
		//printf(".");
		alarm(1);
		return;
	}

	seeker_done = 1;

};

int seeker(struct disk_stat * r, const char * devname, const int disk_io_size[DISK_NUM_IO_TYPES]){
	int fd;
	unsigned long count=0, dev_size=0, block_size=0, num_blocks=0;
	char * buffer;
	off64_t offset=0, offset_saved=0;

	//setvbuf(stdout, NULL, _IONBF, 0);

	/*
	printf("Seeker v2.0, 2007-01-15, "
	       "http://www.linuxinsight.com/how_fast_is_your_disk.html\n");

	if (argc != 2) {
		printf("Usage: seeker <raw disk device>\n");
		exit(EXIT_SUCCESS);
	}*/

	if((fd = open(devname, O_RDONLY)) < 0){
		perror("open");
		return 1;
	}

#if defined(BLKGETSIZE64)
	if(ioctl(fd, BLKGETSIZE64, &dev_size) == -1){
#else
	if(ioctl(fd, BLKGETSIZE, &dev_size) == -1){
#endif
		perror("ioctl");
		close(fd);
		return 1;
	}

	if(ioctl(fd, BLKPBSZGET, &block_size) == -1){
		perror("ioctl");
		close(fd);
		return 1;
	}

	num_blocks = dev_size / block_size;
	r->block_size = block_size;
	r->num_blocks = num_blocks;
	offset = offset_saved = (off64_t) random() % num_blocks;

	int t;
	for(t=0; t < 3; t++){ //random, sequential, cached sequential read

		buffer = (char*) malloc(disk_io_size[t]);
		if(buffer == NULL){
			perror("malloc");
			break;
		}
		int buffer_blocks = disk_io_size[t] / block_size;

		if(t == 1){ //sequential read
			//clear caching, because we want to read directly from disk
			//we also want to put data in cache, since the next measure is cached read
			system("echo 3 > /proc/sys/vm/drop_caches");
		}

		if(t > 0){ //seq and cached seq
			//TODO: start reading from middle of disk, to hav average speed ?
			offset = offset_saved;
			if(lseek64(fd, offset*block_size, SEEK_SET) == (off64_t) -1){
				perror("lseek64");
				break;
			}
		}
		time(&start);
		srand(start);
		signal(SIGALRM, &done);
		alarm(1);

		count = 0;
		seeker_done = 0; //reset

		while(seeker_done == 0){
			if(t == 0){ //if random
				//seek to block
				offset = (off64_t) random() % (num_blocks - buffer_blocks);
				if(lseek64(fd, block_size * offset, SEEK_SET) == (off64_t) -1){
					perror("lseek64");
					break;
				}
			}

			if(read(fd, buffer, buffer_blocks*block_size) < 0){
				perror("read");
				break;
			}
			count += buffer_blocks;

			if(t > 0){
				//if we are at the end, return to start
				if((offset + count) >= dev_size){
					if(lseek64(fd, 0, SEEK_SET) == (off64_t) -1){
						perror("lseek64");
						break;
					}
				}
			}
		}

		if (count) {
			r->seeks[t] 	  = count / SEEKER_TIMEOUT;
			r->access_time[t] = (1000.0f * SEEKER_TIMEOUT) / (float)count;
		}

		free(buffer);
	}
	close(fd);

	return 0;
}
