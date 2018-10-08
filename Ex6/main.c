//#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "io.h"
#include <native/task.h>
#include <native/timer.h>
#include <sys/mman.h>
#include <rtdk.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>


#define PIN_A 1
#define PIN_B 2
#define PIN_C 3
#define NUM_XEN_TASKS 3
#define DIST_THREADS 10

 
static RT_TASK xen_tasks[NUM_XEN_TASKS];
static pthread_t distThreads[DIST_THREADS];


void xen_func_periodic(void *arg)
{
  unsigned int pin = *((int*)(&arg));

  rt_task_set_periodic(NULL, TM_NOW, 1000000);
  while (1) {
    if(!io_read(pin)){
	// Busy-polling that won't crash your computer (hopefully)
	  io_write(pin, 0);
	  //rt_timer_spin(50000);
	  io_write(pin, 1);
	
    }

    rt_task_wait_period(NULL);
  }
  return;
}

void xen_func(void *arg)
{
  unsigned int pin = *((int*)(&arg));
 
  unsigned long duration = 10000000000;  // 10 second timeout
  unsigned long endTime = rt_timer_read() + duration;

  while(1){
	if(!io_read(pin)){
	// Busy-polling that won't crash your computer (hopefully)
	  io_write(pin, 0);
	  //rt_timer_spin(50000);
	  io_write(pin, 1);
	
	}

 	if(rt_timer_read() > endTime){
        	rt_printf("Time expired\n");
        	rt_task_delete(NULL);
    	}
    	if(rt_task_yield()){
       		rt_printf("Task failed to yield\n");
        	rt_task_delete(NULL);
    	}
  }

}

// Set single CPU for pthread threads (NOT Xenomai threads!)

int set_cpu(int cpu_number){
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(cpu_number, &cpu);

	return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu);
}


void* threadFuncDist(void * ptr){
	set_cpu(1);
	while(1){
		asm volatile("" ::: "memory");

	}

	return NULL;
}


int main(int argc, char* argv[])
{
	
  rt_print_auto_init(1);
  mlockall(MCL_CURRENT | MCL_FUTURE);

  io_init();
  
  rt_task_create(&xen_tasks[0], "A", 0, 50, T_CPU(1));
  rt_task_create(&xen_tasks[1], "B", 0, 50, T_CPU(1));
  rt_task_create(&xen_tasks[2], "C", 0, 50, T_CPU(1));

  /*rt_task_start(&xen_tasks[0], &xen_func, (void*)1);
  rt_task_start(&xen_tasks[1], &xen_func, (void*)2);
  rt_task_start(&xen_tasks[2], &xen_func,(void*)3);*/

  rt_task_start(&xen_tasks[0], &xen_func_periodic, (void*)1);
  rt_task_start(&xen_tasks[1], &xen_func_periodic, (void*)2);
  rt_task_start(&xen_tasks[2], &xen_func_periodic,(void*)3);


  //add disturbance
  for(int i = 0; i < DIST_THREADS; i++){
	pthread_create(&distThreads[i], NULL, threadFuncDist, NULL);
  }

  /* Wait for threads to be finished */
  for (int i = 0; i< DIST_THREADS; ++i){
	pthread_join(distThreads[i], NULL);
  }

 while(1){
	sleep(-1);	

}
}
