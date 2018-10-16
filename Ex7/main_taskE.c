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

#define HIGH_A 0
#define HIGH_B 1
#define LOW_A 2
#define LOW_B 3


#define NUM_XEN_TASKS 2
#define NUM_XEN_MUTEX 2

typedef struct resource_t{
	RT_MUTEX mtx;
	unsigned int prio;
	char* mtx_name;
	char* task_name;

} resource_t;

static RT_MUTEX mtx[NUM_XEN_MUTEX];
static RT_SEM semaphore;
static RT_TASK xen_tasks[NUM_XEN_TASKS];
static resource_t resources[4];

void busy_wait_us(unsigned long delay){
	for(; delay > 0; delay--){
		rt_timer_spin(1000);
	}
}


void icpp_lock(resource_t* res){
	rt_printf("%s wants to take %s\r\n", res->task_name, res->mtx_name);

	
	rt_printf("%s with base prio %d has boost prio 40\r\n", res->task_name, res->prio);

	//take mutex 
	rt_mutex_acquire(&res->mtx,TM_INFINITE);

	rt_printf("%s has %s\r\n",res->task_name, res->mtx_name);

}

void icpp_unlock(resource_t* res){
	
	rt_printf("%s releases %s\r\n", res->task_name, res->mtx_name);
	//release resource
	rt_mutex_release(&res->mtx);

	rt_printf("%s has base prio %d\r\n", res->task_name, res->prio);
	
	
}


void xen_funcLOW(void *arg)
{
	rt_task_set_priority(&xen_tasks[1],40);
	icpp_lock(&resources[LOW_A]);
	

	//busy wait for 3 time units
	busy_wait_us(300000);

	icpp_lock(&resources[LOW_B]);

	//busy wait for 3 time units
	busy_wait_us(300000);

	icpp_unlock(&resources[LOW_B]);

	icpp_unlock(&resources[LOW_A]);

	rt_task_set_priority(&xen_tasks[1],20);

	//busy wait for 1 time unit
	busy_wait_us(100000);


}

void xen_funcHIGH(void *arg)
{
	
	rt_task_sleep(100*1000*1000);

	rt_task_set_priority(&xen_tasks[0],40);
	icpp_lock(&resources[HIGH_B]);

	//busy wait for 1 time unit
	busy_wait_us(100000);

	icpp_lock(&resources[HIGH_A]);

	//busy wait for 2 time units
	busy_wait_us(200000);

	icpp_unlock(&resources[HIGH_A]);

	icpp_unlock(&resources[HIGH_B]);
	
	rt_task_set_priority(&xen_tasks[0],30);

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

	resources[0].mtx = mtx[0];
	resources[0].prio = 30;
	resources[0].mtx_name = "Mutex A";
	resources[0].task_name = "HIGH";

	resources[1].mtx = mtx[1];
	resources[1].prio = 30;
	resources[1].mtx_name = "Mutex B";
	resources[1].task_name = "HIGH";

	resources[2].mtx = mtx[0];
	resources[2].prio = 20;
	resources[2].mtx_name = "Mutex A";
	resources[2].task_name = "LOW";

	resources[3].mtx = mtx[1];
	resources[3].prio = 20;
	resources[3].mtx_name = "Mutex B";
	resources[3].task_name = "LOW";

	//create two tasks
	rt_task_create(&xen_tasks[0], "HIGH", 0, 30, T_CPU(1));
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

