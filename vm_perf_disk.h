/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#ifndef VM_PERF_DISK_H
#define VM_PERF_DISK_H

// Random, sequential, cached_sequential
#define DISK_NUM_IO_TYPES 3

struct disk_stat{
	char devname[10];			/// /dev/sda, /dev/vda, /dev/hda, ...
	unsigned long num_blocks;
	unsigned long block_size;

	// Read data
	int seeks[DISK_NUM_IO_TYPES];
	float access_time[DISK_NUM_IO_TYPES];
};

struct disk_result{
	int num_disks;

	// Rrite data to user HOME directory
	float write[DISK_NUM_IO_TYPES];

	struct disk_stat * disk_stats;
};

void disk_bench(struct disk_result *r);
void disk_report(const struct disk_result *r);

#endif
