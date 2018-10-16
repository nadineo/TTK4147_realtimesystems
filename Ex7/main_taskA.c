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

#define NUM_XEN_TASKS 2

static RT_SEM semaphore;
static RT_TASK xen_tasks[NUM_XEN_TASKS];

void xen_func(void *arg)
{
	unsigned int task = *((int*)(&arg));

	rt_sem_p(&semaphore, TM_INFINITE);

	rt_printf("Task %d got semaphore \r\n",task);
	
	while(1){

	}

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

	//create semaphore
	rt_sem_create(&semaphore, "sema", 0, S_PRIO);
	
	//create two tasks
	rt_task_create(&xen_tasks[0], "Task1", 0, 40, T_CPU(1));
	rt_task_create(&xen_tasks[1], "Task2", 0, 30, T_CPU(1));

	rt_task_start(&xen_tasks[0], &xen_func, (void*)1);
	rt_task_start(&xen_tasks[1], &xen_func, (void*)2);

	rt_task_sleep(100*1000*1000);

	rt_sem_broadcast(&semaphore);
	
	rt_task_sleep(100*1000*1000);
	
	rt_sem_delete(&semaphore);
	
	rt_task_delete(&xen_tasks[0]);
	rt_task_delete(&xen_tasks[1]);

	return 0;
}
