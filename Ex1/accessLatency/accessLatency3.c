
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <x86intrin.h>



int main(){
struct timespec start;

/*for(int i = 0; i < 10*1000*1000; i++){
    // read timer
    
   clock_gettime(CLOCK_REALTIME, &start);
   printf("%lld.%.9ld", (long long)start.tv_sec, start.tv_nsec);

}*/


int ns_max = 2000;
int histogram[ns_max];
memset(histogram, 0, sizeof(int)*ns_max);
clock_t ticks = sysconf(_SC_CLK_TCK);

for(int i = 0; i < 10*1000*1000; i++){
    
    long time1 = __rdtsc();
    sched_yield();
    long time2 = __rdtsc();
    
    long ns = (time2-time1);

    
    if(ns >= 0 && ns < ns_max){
        histogram[ns]++;
    }
}

for(int i = 0; i < ns_max; i++){
    printf("%d\n", histogram[i]);
}


	return 0;
}



