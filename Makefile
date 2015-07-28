CC=gcc
#CFLAGS=-D_GNU_SOURCE -Wall -O2 -march=native -mtune=native
CFLAGS=-D_GNU_SOURCE -Wall -ggdb
LDFLAGS=-lrt -lm -lpthread -fopenmp -lresolv

DEP=c-ray.o dhry.o stream.o seeker.o
OBJECTS=vm_perf.o vm_perf_net.o vm_perf_cpu.o vm_perf_mem.o vm_perf_disk.o vm_perf_sys.o $(DEP)

vm_perf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o vm_perf $(LDFLAGS)

vm_perf.o: vm_perf.c vm_perf.h
	$(CC) $(CFLAGS) -c vm_perf.c

vm_perf_net.o: vm_perf_net.c vm_perf_net.h
	$(CC) $(CFLAGS) -c vm_perf_net.c

vm_perf_cpu.o: vm_perf_cpu.c vm_perf_cpu.h dhry.o c-ray.o
	$(CC) $(CFLAGS) -c vm_perf_cpu.c

vm_perf_mem.o: vm_perf_mem.c vm_perf_mem.h stream.o
	$(CC) $(CFLAGS) -c vm_perf_mem.c

vm_perf_disk.o: vm_perf_disk.c vm_perf_disk.h seeker.o
	$(CC) $(CFLAGS) -c vm_perf_disk.c

vm_perf_sys.o: vm_perf_sys.c vm_perf_sys.h
	$(CC) $(CFLAGS) -c vm_perf_sys.c

#External source
c-ray.o: dep/c-ray.c dep/c-ray.h
	$(CC) $(CFLAGS) -O3 -ffast-math -c dep/c-ray.c

dhry.o: dep/dhry_1.c dep/dhry_2.c dep/dhry.h
	$(CC) $(CFLAGS) -c dep/dhry_1.c
	$(CC) $(CFLAGS) -c dep/dhry_2.c
	ld -r -o dhry.o dhry_1.o dhry_2.o

stream.o: dep/stream.c dep/stream.h
	$(CC) $(CFLAGS) -fopenmp -O3 -c dep/stream.c

seeker.o: dep/seeker.c dep/seeker.h
	$(CC) $(CFLAGS) -O2 -c dep/seeker.c

clean:
	rm -f $(OBJECTS) vm_perf *.o


archive:
	tar -cjf vm_perf-x.x.tar.bz2 dep Makefile README.txt vm_perf.{c,h} vm_perf_{cpu,disk,mem,net,sys}.c vm_perf_{cpu,disk,mem,net,sys}.h

memcheck:
	valgrind -v --tool=memcheck \
			--leak-check=full 	 \
			--show-reachable=yes \
			--track-origins=yes  \
		--log-file=vm_perf.log ./vm_perf
