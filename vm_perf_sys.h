/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#ifndef VM_PERF_SYS_H
#define VM_PERF_SYS_H
#include <limits.h>

#define NUM_INFO_PATHS 2

struct sys_result{
	char * info[NUM_INFO_PATHS];
	char hostname[HOST_NAME_MAX];
	char cpu_model[100];
	unsigned short cpu_count;
	unsigned long totalRAM;
	unsigned long freeRAM;
};

void sys_info(struct sys_result * r);
void sys_report(const struct sys_result * r);

#endif
