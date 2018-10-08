#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

/* Task C: uncomment this to run with semaphore */
#define WITH_SEMAPHORE

static unsigned long sharedVar = 0;
static sem_t semaphore;


// Note the argument and return types: void*
void* incrementFunc(void* id){
    int threadId = *((int*)id);
    unsigned long cnt = 0;
    for (int i = 0; i< 50000000;i++){
        cnt++;
        /* Task C */
        #ifdef WITH_SEMAPHORE
            sem_wait(&semaphore);
            sharedVar++;
            sem_post(&semaphore);
        #else
            sharedVar++;
        #endif
    }

    /* in order to synch output as well */
    /* #ifdef WITH_SEMAPHORE
        sem_wait(&semaphore);
        printf("Thread %d: cnt: %ld, sharedVar: %ld\n", threadId, cnt, sharedVar);
        sem_post(&semaphore);
    #else 
        printf("Thread %d: cnt: %ld, sharedVar: %ld\n", threadId, cnt, sharedVar);
    #endif*/

    return NULL;
}



int main(){

    int pShared = 0;
    int value = 1;

    #ifdef WITH_SEMAPHORE
        sem_init(&semaphore, pShared, value);
    #endif

    int threadId1 = 1;
    int threadId2 = 2;

    pthread_t threadHandle1;
    pthread_t threadHandle2;

    pthread_create(&threadHandle1, NULL, incrementFunc, (void*)&threadId1);
    pthread_create(&threadHandle2, NULL, incrementFunc, (void*)&threadId2);

    pthread_join(threadHandle1, NULL);
    pthread_join(threadHandle2, NULL);
}





/*#include <pthread.h>

int main(){
    pthread_mutex_t mtx;

    // 2nd arg is a pthread_mutexattr_t
    pthread_mutex_init(&mtx, NULL);

    pthread_mutex_lock(&mtx);
    // Critical section
    pthread_mutex_unlock(&mtx);

    pthread_mutex_destroy(&mtx);
}*/