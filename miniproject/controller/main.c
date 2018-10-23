#include "communication.h"
#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "time.h"


#define KP 10
#define KI 800
#define SAMPLE_TIME 0.001

//receive thread -> handles incoming messages
static pthread_t threadRecv;

//thread that sends the acknowledge to server
static pthread_t threadSendAck;

//thread that performs the controller logic
static pthread_t threadController;

static pthread_mutex_t mtxBuffer;

//shared resource for acknowledge -> semaphore
static sem_t semaAck;



static const char* STR_SIGNAL = "SIGNAL";
static const char* STR_GET_ACK = "GET_ACK:";




// ------------------------------------------------
// thread function for receiving messages 
// ------------------------------------------------
void* recv_func(void* param){
	char recvBuf[64];
	memset(recvBuf, 0, sizeof(recvBuf));
	memset(yValBuf, 0, sizeof(vValBuf));
	char delimiter[] = ":";
	char* ptr;
	float y = 0.0f;

	receive_msg(recvBuf);



	//check if SIGNAL is sent
	if(strcmp(recvBuf, STR_SIGNAL, strlen(STR_SIGNAL)) == 0){
		//increment semaphore
		sem_post(&semaphore);
	}
	else if(strcmp(recvBuf, STR_GET_ACK, strlen(STR_GET_ACK)) == 0){
		
		ptr = strtok(recvBuf, delimiter);
		while(ptr != NULL){
			//get second entry of get value after ':'
			ptr = strtok(NULL, delimiter);
			//convert to float value
			y = atof(ptr);

		}
		
		//put item in buffer
		put_item_in_buffer(y);

	}

	

}



void* thread_controller_func(){
	int cnt = 0;
	float dummy = 0.0f;
	float y;

	struct timespec next;
	clock_gettime(CLOCK_REALTIME, &next);

	//send start to server
	send_msg(eStart, dummy);

	//TODO > how to iterate
	while(cnt < iterations){
		//send get
		send_msg(eGet, dummy);

		//wait on receiving the y val from the server
		y = get_item_from_buffer();

		//do calculations 

		//
		//
		//
		//

	}

}
// ------------------------------------------------
// thread function for sending acknowledge
// ------------------------------------------------
void* send_ack_func(void* param){
	float dummy = 0.0f;

	while(1){
		//send acknowledge only if a SIGNAL message
		//has arrived -> semaphore counter has been incremented
		sem_wait(&semaAck);
		send_msg(eAck,dummy);
	}

	return NULL;
    
}


int main(){
	int pShared = 0;
    int value = 0;

	sem_init(&semaAck, pShared, value);













	return 0;
}