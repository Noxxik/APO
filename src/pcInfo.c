#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/statvfs.h>

typedef struct {
	int user, nice, system, idle, iowait, irq, softirq;
} LoadInfo;

typedef struct {
	double frequency, load;
} CpuInfo;

typedef struct {
	unsigned long long space, free;
} DeviceInfo;

unsigned long long rdtsc();
unsigned long long getDiffInMiliseconds(struct timeval * start, struct timeval * end);
void getLoadInfo(LoadInfo * loadInfo);
long long sumAll(LoadInfo * loadInfo);
long long sumWork(LoadInfo * loadInfo);
void getCpuInfo(CpuInfo * ci, int measureTime);
void getDiscInfo(DeviceInfo * di, char * file);
void getMemoryInfo(DeviceInfo * di);


unsigned long long rdtsc() {
    unsigned long long d;
    __asm__ __volatile__ ("rdtsc" : "=A" (d) );
    return d;
}


unsigned long long getDiffInMiliseconds(struct timeval * start, struct timeval * end) {
	return (end->tv_sec - start->tv_sec)*1000 +
		   (end->tv_usec - start->tv_usec)/1000;
}

void getLoadInfo(LoadInfo * loadInfo) {
	FILE * stat;
	stat= fopen("/proc/stat", "r");
	fscanf(stat, "cpu %d %d %d %d %d %d %d",
			&(loadInfo->user),
			&(loadInfo->nice),
			&(loadInfo->system),
			&(loadInfo->idle),
			&(loadInfo->iowait),
			&(loadInfo->irq),
			&(loadInfo->softirq));
	fclose(stat);
}

long long sumAll(LoadInfo * loadInfo) {
	return loadInfo->user + loadInfo->nice + loadInfo->system + loadInfo->idle
			+ loadInfo->iowait + loadInfo->irq + loadInfo->softirq;
}


long long sumWork(LoadInfo * loadInfo) {
	return loadInfo->user + loadInfo->nice + loadInfo->system;
}


void getCpuInfo(CpuInfo * ci, int measureTime) {
	struct timeval s,e;
	LoadInfo liStart, liEnd;
	unsigned long long rStart, rEnd;
	getLoadInfo(&liStart);
	rStart = rdtsc();
	gettimeofday(&s, NULL);
	usleep(measureTime);
	rEnd = rdtsc();
	getLoadInfo(&liEnd);
	gettimeofday(&e, NULL);
	double time = getDiffInMiliseconds(&s, &e);
	ci->frequency = (rEnd-rStart)/(1000*time);
	ci->load = (sumWork(&liEnd) - sumWork(&liStart))/(double)(sumAll(&liEnd) - sumAll(&liStart));
}

void getDiscInfo(DeviceInfo * di, char * file) {
	struct statvfs buf;
	if (!statvfs(file, &buf)) {
		di->space = buf.f_blocks*(unsigned long long)buf.f_bsize;
		di->free = buf.f_bfree*(unsigned long long)buf.f_bsize;
	} else {
		puts("Error when accessing disc info\n");
	}
}


void getMemoryInfo(DeviceInfo * di) {
	FILE * mi;
	mi = fopen("/proc/meminfo", "r");
	fseek(mi, 17, SEEK_SET);
	fscanf(mi, "%lld", &(di->space));
	fseek(mi, 17+28, SEEK_SET);
	fscanf(mi, "%lld", &(di->free));
	fclose(mi);
}


int main(int argc, char** argv) {
	CpuInfo cpuInfo;
	DeviceInfo discInfo, memoryInfo;
	getCpuInfo(&cpuInfo, 1000000);
	getDiscInfo(&discInfo, argv[0]);
	getMemoryInfo(&memoryInfo);
	puts(argv[0]);
	printf("CPU frequency: %.3f Mhz\n", cpuInfo.frequency);
	printf("CPU load: %.3f %% \n", 100*cpuInfo.load);
	printf("Disc size: %.1f/%.1f GB\n", discInfo.free/1000000000., discInfo.space/1000000000.);
	printf("Memory size: %.1f/%.1f MB\n", memoryInfo.free/1000., memoryInfo.space/1000.);
	return EXIT_SUCCESS;
}
