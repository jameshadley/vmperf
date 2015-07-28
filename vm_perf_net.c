/*
 * Copyright 2015 Loading Deck Limited - https://www.loadingdeck.com/
 * Use of this program or parts thereof is subject to the GPLv3 licence
 */

#include <time.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "vm_perf_net.h"

// http://www.softlayer.com/data-centers
const char * net_test_domains[NET_NUM_DOMAINS] = {
	"speedtest.ams01.softlayer.com",
	"speedtest.dal01.softlayer.com",
	"speedtest.fra02.softlayer.com",
	"speedtest.hkg02.softlayer.com",
	"speedtest.lon02.softlayer.com",
	"speedtest.mel01.softlayer.com",
	"speedtest.par01.softlayer.com",
	"speedtest.mex01.softlayer.com",
	"speedtest.sjc01.softlayer.com",
	"speedtest.tok02.softlayer.com",
	"speedtest.tor01.softlayer.com",
	"speedtest.wdc01.softlayer.com"
};

// Number of ICMP packets to send before determining the latency
#define NET_MAX_PINGS 10
#define MSG_SIZE	24

struct packet{
	struct icmphdr hdr;
	char msg[MSG_SIZE];
};

struct reply_packet{
	struct iphdr iph;
	struct icmphdr icmphdr;
	char msg[MSG_SIZE];
};

static unsigned short checksum(void *b, int len){
	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

static float net_test_latency(const char * hostname){

	struct protoent *proto = getprotobyname("ICMP");
	struct hostent * hname = gethostbyname(hostname);
	if(hname == NULL){
		perror("gethostbyname");
		return 0.0f;
	}

	const int val=255;
	int i, sd;
	unsigned short cnt = 0;
	struct packet pckt;
	struct sockaddr_in addr;
	struct timespec ts_start[NET_MAX_PINGS], ts_end[NET_MAX_PINGS];

	bzero(ts_start, sizeof(ts_start));
	bzero(ts_end, sizeof(ts_end));
	bzero(&addr, sizeof(addr));

	addr.sin_family = hname->h_addrtype;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = *(long*)hname->h_addr_list[0];

	sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
	if ( sd < 0 ){
		perror("socket");
		return 0.0f;
	}

	if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
		perror("Set TTL option");

	if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");

	// Send ICMP packets
	bzero(&pckt, sizeof(pckt));

	pckt.hdr.type = ICMP_ECHO;
	pckt.hdr.un.echo.id = 1313;

	pid_t pid = fork();
	if(pid == 0){
		for (i=0; i < NET_MAX_PINGS; ++i){
			clock_gettime(CLOCK_REALTIME, (struct timespec*)&pckt.msg);// &ts_start[i]);

			pckt.hdr.un.echo.sequence = cnt++;
			// memcpy(pckt.msg, &ts_start[i], sizeof(struct timespec));
			pckt.hdr.checksum = 0;
			pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

			if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&addr, sizeof(addr)) <= 0 )
				perror("sendto");
			usleep(100);
		}
		exit(0); //
	}else if(pid == -1){
		perror("fork");
		return 0.0f;
	}

	// Receive replies
	struct reply_packet reply;
	for (i=0; i < NET_MAX_PINGS; ++i){

		fd_set rfds;
        struct timeval tv;

		FD_ZERO(&rfds);
        FD_SET(sd, &rfds);

        tv.tv_sec = 1*NET_MAX_PINGS;
        tv.tv_usec = 0;

		int retval = select(sd+1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now */

	   if (retval == -1){
		   perror("select()");
		   break;
	   }else if (retval){
		   //printf("Data is available now.\n");
		   /* FD_ISSET(sd, &rfds) will be true. */
	   }else{
		   // fprintf(stderr, "No data within five seconds.\n");
		   break;
	   }

		bzero(&reply, sizeof(reply));
		socklen_t len = sizeof(addr);
		int bytes = recvfrom(sd, &reply, sizeof(reply), 0, (struct sockaddr*)&addr, &len);
		if ( bytes < 0 ){
			if((errno == EAGAIN) && (cnt > 0)){
				--i;
				continue;
			}else{
				perror("recvfrom");
				break;
			}
		}


		if ( reply.icmphdr.un.echo.id == 1313 )
		{
			//printf("ICMP: type[%d/%d] checksum[%d] id[%d] seq[%d]\n",
			//	reply.icmphdr.type, reply.icmphdr.code, ntohs(reply.icmphdr.checksum),
			//	reply.icmphdr.un.echo.id, reply.icmphdr.un.echo.sequence);

			unsigned short idx = reply.icmphdr.un.echo.sequence;
			if((idx >= 0) && (idx < NET_MAX_PINGS)){
				clock_gettime(CLOCK_REALTIME, &ts_end[idx]);
				memcpy(&ts_start[idx], reply.msg, sizeof(struct timespec));
			}else{
				//fprintf(stderr, "Error: ping with wrong idx\n");
			}
		}
		--cnt;
	}

	// Calculate latency
	float latency_ms = 0.0f;
	int num_samples = 0;
	for (i=0; i < NET_MAX_PINGS; ++i){
		if(ts_end[i].tv_nsec == 0)
			continue;
		ts_end[i].tv_sec  -= ts_start[i].tv_sec;
		ts_end[i].tv_nsec -= ts_start[i].tv_nsec;

		float lat = ((float)ts_end[i].tv_sec / 100) + ((float)ts_end[i].tv_nsec / 1000000);

		latency_ms += lat;
		num_samples++;
	}
	if(num_samples > 0)
		latency_ms /= num_samples;

	return latency_ms;
};

static float net_test_dns_query(const char * hostname){

	struct timespec ts_start, ts_end;
	unsigned char answer[1024];

	clock_gettime(CLOCK_REALTIME, &ts_start);
	if(res_query(net_test_domains[0], C_IN, T_MX, answer, sizeof(answer)) == -1){
		// perror("res_query");
		return 0.0f;
	}
	clock_gettime(CLOCK_REALTIME, &ts_end);

	ts_end.tv_sec  -= ts_start.tv_sec;
	ts_end.tv_nsec -= ts_start.tv_nsec;

	return ((float)ts_end.tv_sec / 100) + ((float)ts_end.tv_nsec / 1000000);
}


void net_bench(struct net_result * r){
	bzero(r, sizeof(struct net_result));

	int i;
	for(i=0; i < NET_NUM_DOMAINS; i++){
		r->latency[i]   = net_test_latency(net_test_domains[i]);
		r->dns_query[i] = net_test_dns_query(net_test_domains[i]);
	}
};

void net_report(const struct net_result * r){
	printf("\"net\":[");
	int i;
	char delim = ' ';
	for(i=0; i < NET_NUM_DOMAINS; ++i){
		printf("%c{\"hostname\":\"%s\",\"latency\":\"%.2fms\",\"dns_query\":\"%.2fms\"}",
				delim, net_test_domains[i], r->latency[i], r->dns_query[i]);
		delim = ',';
	}
	printf("]");
};
