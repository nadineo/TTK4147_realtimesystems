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



int main(int argc, char const *argv[])
{
	struct tms st_cpu;

    

	/*for(int i = 0; i < 10*1000*1000; i++){
    // read timer
    
   		times(&st_cpu);

}*/
	int ns_max = 50;
	int histogram[ns_max];
	memset(histogram, 0, sizeof(int)*ns_max);
	struct tms time1;
	struct tms time2;
	clock_t ticks = sysconf(_SC_CLK_TCK);

	for(int i = 0; i < 10*1000*1000; i++){
    
    	times(&time1);
    	times(&time2);
    
    	int ns = (time2.tms_utime-time1.tms_utime)/ticks * 1000000000; // (t2 - t1) * ??
    	printf("ns: %lu\n", ns);
    
    	if(ns >= 0 && ns < ns_max){
        	histogram[ns]++;
    	}
	}

	for(int i = 0; i < ns_max; i++){
   	 	printf("%d\n", histogram[i]);
	}


	return 0;
}




