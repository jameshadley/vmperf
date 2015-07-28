/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#ifndef VM_PERF_NET_H
#define VM_PERF_NET_H

#define NET_NUM_DOMAINS 12

struct net_result{
	unsigned char network_capacity;	// Megabits/s
	float latency[NET_NUM_DOMAINS];		// Latency
	float dns_query[NET_NUM_DOMAINS];	// Time taken for a single dns query
};

void net_bench(struct net_result * r);
void net_report(const struct net_result * r);
#endif
