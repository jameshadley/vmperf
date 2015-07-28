#ifndef SEEKER_H
#define SEEKER_H

#include "../vm_perf_disk.h"

#define SEEKER_TIMEOUT 10

int seeker(struct disk_stat * r, const char * devname, const int disk_io_size[DISK_NUM_IO_TYPES]);

#endif
