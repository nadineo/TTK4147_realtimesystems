
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sched.h>




int main(){
struct timespec start;

/*for(int i = 0; i < 10*1000*1000; i++){
    // read timer
    
   clock_gettime(CLOCK_REALTIME, &start);
   printf("%lld.%.9ld", (long long)start.tv_sec, start.tv_nsec);

}*/


int ns_max = 1000;
int histogram[ns_max];
memset(histogram, 0, sizeof(int)*ns_max);
struct timespec time1;
struct timespec time2;

for(int i = 0; i < 10*1000*1000; i++){
    
    clock_gettime(CLOCK_MONOTONIC, &time1);
    sched_yield();
    clock_gettime(CLOCK_MONOTONIC, &time2);
    
    int ns = (time2.tv_sec-time1.tv_sec)*1000000000 + (time2.tv_nsec - time1.tv_nsec); // (t2 - t1) * ??
    
    if(ns >= 0 && ns < ns_max){
        histogram[ns]++;
    }
}

for(int i = 0; i < ns_max; i++){
    printf("%d\n", histogram[i]);
}


	return 0;
}



