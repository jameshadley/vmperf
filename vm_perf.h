/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */
#ifndef VM_PERF_H
#define VM_PERF_H

#define VERSION "0.4"

#include "vm_perf_net.h"
#include "vm_perf_cpu.h"
#include "vm_perf_mem.h"
#include "vm_perf_disk.h"
#include "vm_perf_sys.h"

struct vm_perf_result{
	struct sys_result sys;
	struct cpu_result cpu;
	struct net_result net;
	struct mem_result mem;
	struct disk_result disk;
};
#endif
