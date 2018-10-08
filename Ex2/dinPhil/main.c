#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>


#define NUM_PHILOSOPHERS 5
#define NUM_MEALS 10

static pthread_t philThreads[NUM_PHILOSOPHERS];
static pthread_mutex_t forksMutex[NUM_PHILOSOPHERS];

/* Thread function of philosopher:
	eating - thinking 
 */
void* philFunc(void* id){
	int philId = *((int*)(id));
	int neighbourID = (philId+1)%NUM_PHILOSOPHERS;
	int cntMeals = 0;
	
	/* every philosophers will have a specified number of meals */
	while(cntMeals < NUM_MEALS){
		/* in order to eat he has to get two forks (two mutexes) */
		if(pthread_mutex_trylock(&forksMutex[philId]) == 0){
			//philosopher has his own fork - now he has to get the one of his neighbour
			if(pthread_mutex_trylock(&forksMutex[neighbourID])== 0){
				//he has now both forks -> can eat now
				printf("philosopher %d is eating\n", philId);
				sleep(1);
				//release for from neighbour
				pthread_mutex_unlock(&forksMutex[neighbourID]);
			}
			//release own fork
			pthread_mutex_unlock(&forksMutex[philId]);
			printf("philosopher %d is thinking\n", philId);
		}
		sleep(1);
		cntMeals++;
	}

	printf("\nphilosopher %d has finished!\n", philId);
	return NULL;
}



int main(){

	int threadIds[NUM_PHILOSOPHERS];

	/* First create mutexes in order to avoid race conditions when philosophers want to
   	access other forks */
	for(int i = 0; i< NUM_PHILOSOPHERS; ++i){
		pthread_mutex_init(&forksMutex[i], NULL);
	}

	/* Create five threads to represent each philosopher */
	for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
	{
		threadIds[i] = i;
		pthread_create(&philThreads[i], NULL, philFunc, (void *)&threadIds[i]);
	}

	/* Wait for every philosopher to be finished */
	for (int i = 0; i< NUM_PHILOSOPHERS; ++i){
		pthread_join(philThreads[i], NULL);
	}

	for (int i = 0; i < NUM_PHILOSOPHERS; ++i)
	{
		/* destroy mutexes */
		pthread_mutex_destroy(&forksMutex[i]);
	}

	return 0; 
}
