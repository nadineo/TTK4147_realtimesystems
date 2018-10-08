#define _GNU_SOURCE
#include "io.h"
#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>

#define NUM_THREADS 3
#define DIST_THREADS 0

#define PIN_A 1
#define PIN_B 2
#define PIN_C 3





/*void* threadFunc(void* id){
	int testPinID = *((int*)(id));

	//wait until pin gets high
	while(!io_read(testPinID)){}

	io_write(testPinID, 0);

}*/


/// Assigning CPU core ///
int set_cpu(int cpu_number){
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);

	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}

static pthread_t testThreads[NUM_THREADS];
static pthread_t distThreads[DIST_THREADS];


void* threadFuncA(void* ptr){
	//set_cpu(1);
	while(1){
		while(io_read(PIN_A)){}

		io_write(PIN_A, 0);
		usleep(50);
	
		io_write(PIN_A, 1);
	}
	

	return NULL;

}

void* threadFuncB(void* ptr){
	//set_cpu(1);
	while(1){

		while(io_read(PIN_B)){}

		io_write(PIN_B, 0);
		usleep(50);
		io_write(PIN_B, 1);
	}
	

	return NULL;

}

void* threadFuncC(void* ptr ){
	//set_cpu(1);
	while(1){

		while(io_read(PIN_C)){}

		io_write(PIN_C, 0);
		usleep(50);
		io_write(PIN_C, 1);
	}

	return NULL;

}

/// 'struct timespec' functions ///

struct timespec timespec_normalized(time_t sec, long nsec){
    while(nsec >= 1000000000){
        nsec -= 1000000000;
        ++sec;
    }
    while(nsec < 0){
        nsec += 1000000000;
        --sec;
    }
    return (struct timespec){sec, nsec};
}

struct timespec timespec_add(struct timespec lhs, struct timespec rhs){
    return timespec_normalized(lhs.tv_sec + rhs.tv_sec, lhs.tv_nsec + rhs.tv_nsec);
}



void* threadFuncPeriodicA(void* ptr){
	set_cpu(1);
	struct timespec waketime;
	clock_gettime(CLOCK_REALTIME, &waketime);

	struct timespec period = {.tv_sec = 0, .tv_nsec = 1*1000*1000};

	while(1){
    	// do periodic work ...
		if(!io_read(PIN_A)){
			io_write(PIN_A, 0);
			usleep(50);
			io_write(PIN_A, 1);
		}
    
    	// sleep
    	waketime = timespec_add(waketime, period);
    	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &waketime, NULL);
    }
    return NULL;
}

void* threadFuncPeriodicB(void* ptr){
	set_cpu(1);
	struct timespec waketime;
	clock_gettime(CLOCK_REALTIME, &waketime);

	struct timespec period = {.tv_sec = 0, .tv_nsec = 1*1000*1000};

	while(1){
    	// do periodic work ...
		if(!io_read(PIN_B)){
			io_write(PIN_B, 0);
			usleep(50);
			io_write(PIN_B, 1);
		}
    
    	// sleep
    	waketime = timespec_add(waketime, period);
    	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &waketime, NULL);
    }
    return NULL;
}

void* threadFuncPeriodicC(void* ptr){
	set_cpu(1);
	struct timespec waketime;
	clock_gettime(CLOCK_REALTIME, &waketime);

	struct timespec period = {.tv_sec = 0, .tv_nsec = 1*1000*1000};

	while(1){
    	// do periodic work ...
		if(!io_read(PIN_C)){
			io_write(PIN_C, 0);
			usleep(50);
			io_write(PIN_C, 1);
		}
    
    	// sleep
    	waketime = timespec_add(waketime, period);
    	clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &waketime, NULL);
    }
    return NULL;
}


	

void* threadFuncDist(void * ptr){
	set_cpu(1);
	while(1){
		asm volatile("" ::: "memory");

	}

	return NULL;
}



int main(){


	//int threadIds[NUM_THREADS]; 

	io_init();

	/* Create three threads 
	for (int i = 0; i < NUM_THREADS; ++i)
	{
		threadIds[i] = i+1;
		pthread_create(&testThreads[i], NULL, threadFunc, (void *)&threadIds[i]);
	}*/

	pthread_create(&testThreads[0], NULL, threadFuncA, NULL);
	pthread_create(&testThreads[1], NULL, threadFuncB, NULL);
	pthread_create(&testThreads[2], NULL, threadFuncC, NULL);

	
	pthread_create(&testThreads[1], NULL, threadFuncPeriodicB, NULL);
	pthread_create(&testThreads[2], NULL, threadFuncPeriodicC, NULL);
	pthread_create(&testThreads[0], NULL, threadFuncPeriodicA, NULL);
	

	//add disturbance
	for(int i = 0; i < DIST_THREADS; i++){
		pthread_create(&distThreads[i], NULL, threadFuncDist, NULL);
	}

	/* Wait for threads to be finished */
	for (int i = 0; i< NUM_THREADS; ++i){
		pthread_join(testThreads[i], NULL);
	}

	/* Wait for threads to be finished */
	for (int i = 0; i< DIST_THREADS; ++i){
		pthread_join(distThreads[i], NULL);
	}



	return 0;
}