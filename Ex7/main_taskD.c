#include <rtdk.h>
#include <native/sem.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <native/task.h>
#include <native/timer.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <native/mutex.h>

#define NUM_XEN_TASKS 2
#define NUM_XEN_MUTEX 2

static RT_MUTEX mtx[NUM_XEN_MUTEX];
static RT_SEM semaphore;
static RT_TASK xen_tasks[NUM_XEN_TASKS];

void busy_wait_us(unsigned long delay){
	for(; delay > 0; delay--){
		rt_timer_spin(1000);
	}
}


void xen_funcLOW(void *arg)
{

	rt_printf("LOW wants to take mutex A\r\n");
	//take mutex A
	rt_mutex_acquire(&mtx[0],TM_INFINITE);

	rt_printf("LOW has mutex A\r\n");

	//busy wait for 3 time units
	busy_wait_us(300000);

	rt_printf("LOW wants to take mutex B\r\n");
	//take mutex B
	rt_mutex_acquire(&mtx[1],TM_INFINITE);
	rt_printf("LOW has mutex B\r\n");

	//busy wait for 3 time units
	busy_wait_us(300000);

	rt_printf("LOW releases mutex B\r\n");
	//release resource
	rt_mutex_release(&mtx[1]);

	rt_printf("LOW releases mutex A\r\n");
	//release resource
	rt_mutex_release(&mtx[0]);

	//busy wait for 1 time unit
	busy_wait_us(100000);




}

void xen_funcHIGH(void *arg)
{

	rt_task_sleep(100*1000*1000);

	rt_printf("HIGH wants to take mutex B\r\n");
	//take mutex B
	rt_mutex_acquire(&mtx[1],TM_INFINITE);

	rt_printf("HIGH has mutex B\r\n");

	//busy wait for 1 time unit
	busy_wait_us(100000);

	rt_printf("HIGH wants to take mutex A\r\n");
	//take mutex A
	rt_mutex_acquire(&mtx[0],TM_INFINITE);
	rt_printf("HIGH has mutex A\r\n");

	//busy wait for 2 time units
	busy_wait_us(200000);

	rt_printf("HIGH releases mutex A\r\n");
	//release resource
	rt_mutex_release(&mtx[0]);

	rt_printf("HIGH releases mutex B\r\n");
	//release resource
	rt_mutex_release(&mtx[1]);

	//busy wait for 1 time unit
	busy_wait_us(100000);
}


int main(){

	rt_print_auto_init(1);
 	mlockall(MCL_CURRENT | MCL_FUTURE);


	rt_task_shadow(NULL, "main", 50, T_CPU(1));

	/*start two tasks
	sleep 100ms
	broadcast semaphore
	sleep 100ms
	delete semaphore
	exit program*/

	//create semaphore for synch
	rt_sem_create(&semaphore, "sema", 0, S_PRIO);

	//create mutex as resource
	rt_mutex_create(&mtx[0], "MutexA");
	rt_mutex_create(&mtx[1], "MutexB");

	//create two tasks
	rt_task_create(&xen_tasks[0], "HIGH", 0, 40, T_CPU(1));
	rt_task_create(&xen_tasks[1], "LOW", 0, 20, T_CPU(1));

	rt_task_start(&xen_tasks[0], &xen_funcHIGH, (void*)1);
	rt_task_start(&xen_tasks[1], &xen_funcLOW, (void*)2);

	
	rt_task_sleep(100*1000*1000);
	
	//start all tasks at the same time
	rt_sem_broadcast(&semaphore);
	
	rt_task_sleep(2*1000*1000*1000);
	
	
	/*rt_sem_delete(&semaphore);
	rt_mutex_delete(&mtx[0]);
	rt_mutex_delete(&mtx[1]);


	rt_task_delete(&xen_tasks[0]);
	rt_task_delete(&xen_tasks[1]);*/

	while(1){
			sleep(-1);
	}
	
	return 0;
}
