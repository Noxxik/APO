#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/statvfs.h>

typedef struct {
	int user, nice, system, idle, iowait, irq, softirq;
} LoadInfo;

unsigned long long rdtsc();
unsigned long long getDiffInMiliseconds(struct timeval * start, struct timeval * end);
void getLoadInfo(LoadInfo * loadInfo);
long long sumAll(LoadInfo * loadInfo);
long long sumWork(LoadInfo * loadInfo);
double getCpuFrequency();
double getCpuLoad(int measureTime);
unsigned long long getDiscSize(char* file);
unsigned long long getFreeDiscSize(char* file);


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


/**
 * Gets frequency in Mhz. Works fine according to /proc/cpuinfo.
 * @param measureTime time to measure in microseconds. The longer the measurement
 * the more accurate the results
 */
double getCpuFrequency(int measureTime) {
	struct timeval s,e;
	unsigned long long start = rdtsc();
	gettimeofday(&s, NULL);
	usleep(measureTime);
	gettimeofday(&e, NULL);
	unsigned long long end = rdtsc();
	double time = getDiffInMiliseconds(&s, &e);
	return (end-start)/(1000*time);
}


/**
 * Gets the load of CPU over the period.
 * @param measureTime time to measure in microseconds.
 */
double getCpuLoad(int measureTime) {
	LoadInfo start, end;
	getLoadInfo(&start);
	usleep(measureTime);
	getLoadInfo(&end);
	return (sumWork(&end) - sumWork(&start))/(double)(sumAll(&end) - sumAll(&start));

}

/**
 * Retrieves the disc size in Bytes.
 * @param file any file on the filesystem in question.
 */
unsigned long long getDiscSize(char* file) {
	struct statvfs buf;
	if (!statvfs(file, &buf)) {
		return buf.f_blocks*(unsigned long long)buf.f_bsize;
	} else {
		puts("Error when accessing disc info\n");
		return 0;
	}
}


/**
 * Retrieves the free disc size in Bytes.
 * @param file any file on the filesystem in question.
 */
unsigned long long getFreeDiscSize(char* file) {
	struct statvfs buf;
	if (!statvfs(file, &buf)) {
		return buf.f_bfree*(unsigned long long)buf.f_bsize;
	} else {
		puts("Error when accessing disc info\n");
		return 0;
	}
}


int main(int argc, char** argv) {
	puts(argv[0]);
	printf("CPU frequency: %.3f Mhz\n", getCpuFrequency(1000000));
	printf("CPU load: %.3f %% \n", 100*getCpuLoad(1000000));
	printf("Disc size: %.1f/%.1f GB\n", getFreeDiscSize(argv[0])/1000000000., getDiscSize(argv[0])/1000000000.);
	return EXIT_SUCCESS;
}
