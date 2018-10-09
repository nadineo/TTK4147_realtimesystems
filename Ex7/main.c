#include <rtdk.h>

#define NUM_XEN_TASKS 2

static RT_TASK xen_tasks[NUM_XEN_TASKS];

void xen_func(void *arg)
{
  unsigned int task = *((int*)(&arg));



}

int main(){

	rt_print_auto_init(1);

	rt_task_shadow(NULL, "main", 0, 50, T_CPU(1));

	/*start two tasks
	sleep 100ms
	broadcast semaphore
	sleep 100ms
	delete semaphore
	exit program*/

	//create semaphore

	rt_task_create(&xen_tasks[0], "Task1", 0, 40, T_CPU(1));
	rt_task_create(&xen_tasks[1], "Task2", 0, 40, T_CPU(1));

	rt_task_start(&xen_tasks[0], &xen_func, (void*)1);
 	rt_task_start(&xen_tasks[1], &xen_func, (void*)2);


  













	return 0;
}