#include <sys/times.h>
#include <stdio.h>
#include <assert.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>


void busy_wait(int sec){
    struct tms st_cpu;

    times(&st_cpu);
	
    clock_t ticks = sysconf(_SC_CLK_TCK);
    while((st_cpu.tms_utime+st_cpu.tms_stime)/ticks < sec){
    	
        for(int i = 0; i < 10000; i++){}
        times(&st_cpu);
    	
    }
}

int main(int argc, char const *argv[])
{
	/* code */
	
	busy_wait(1);

	return 0;
}

