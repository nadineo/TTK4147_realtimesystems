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

#define NUM_XEN_TASKS 3

static RT_SEM semaphore;
static RT_SEM resource;
static RT_TASK xen_tasks[NUM_XEN_TASKS];


void busy_wait_us(unsigned long delay){
	for(; delay > 0; delay--){
		rt_timer_spin(1000);
	}
}


void xen_funcHIGH(void *arg)
{
	
	rt_sem_p(&semaphore, TM_INFINITE);

	rt_task_sleep(200*1000*1000);

	//lock resource
	rt_sem_p(&resource, TM_INFINITE);

	rt_printf("HIGH has resource\r\n");


	rt_printf("HIGH busy wait\r\n");
	busy_wait_us(200000);

	//release resource
	rt_sem_v(&resource);
	rt_printf("HIGH released resource\r\n");


}

void xen_funcMEDIUM(void *arg)
{

	rt_sem_p(&semaphore, TM_INFINITE);

	rt_task_sleep(100*1000*1000);


	rt_printf("MEDIUM busy wait\r\n");
	
	busy_wait_us(500000);
	
	rt_printf("MEDIUM finish busy wait\r\n");
	
}

void xen_funcLOW(void *arg)
{

	rt_sem_p(&semaphore, TM_INFINITE);
	
	//lock resource
	rt_sem_p(&resource, TM_INFINITE);
	rt_printf("LOW has resource\r\n");


	rt_printf("LOW busy wait\r\n");
	busy_wait_us(300000);

	rt_printf("LOW is releasing resource\r\n");
	//release resource
	rt_sem_v(&resource);

	
	

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
	//create semaphore as resource
	rt_sem_create(&resource, "resource", 1, S_PRIO);
	
	//create two tasks
	rt_task_create(&xen_tasks[0], "HIGH", 0, 40, T_CPU(1));
	rt_task_create(&xen_tasks[1], "MEDIUM", 0, 30, T_CPU(1));
	rt_task_create(&xen_tasks[2], "LOW", 0, 20, T_CPU(1));

	rt_task_start(&xen_tasks[0], &xen_funcHIGH, (void*)1);
	rt_task_start(&xen_tasks[1], &xen_funcMEDIUM, (void*)2);
	rt_task_start(&xen_tasks[2], &xen_funcLOW, (void*)3);

	rt_task_sleep(100*1000*1000);
	
	//start all tasks at the same time
	rt_sem_broadcast(&semaphore);
	
	rt_task_sleep(2*1000*1000*1000);
	
	
	rt_sem_delete(&semaphore);
	rt_sem_delete(&resource);
	
	rt_task_delete(&xen_tasks[0]);
	rt_task_delete(&xen_tasks[1]);
	rt_task_delete(&xen_tasks[2]);

	return 0;
}
