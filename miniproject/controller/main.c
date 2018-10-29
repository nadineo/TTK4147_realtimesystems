#include "communication.h"
#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "time.h"


#define Kp 10
#define Ki 800
#define SAMPLE_TIME 0.001
#define RUNTIME 2
#define S_TO_NS 1000000000

//receive thread -> handles incoming messages
static pthread_t threadRecv;

//thread that sends the acknowledge to server
static pthread_t threadSendAck;

//thread that performs the controller logic
static pthread_t threadController;


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
	char getValues[2];
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
		uint8_t i = 0;
		while(ptr != NULL && i < 2){
			getValues[i] = ptr;
			//get second entry of get value after ':'
			ptr = strtok(NULL, delimiter);

			i++;

		}

		//convert second entry of the read SET command to float value
		y = atof(getValues[1]);

		//put item in buffer
		put_item_in_buffer(y);

	}

	

}



void* thread_controller_func(){
int cnt = 0;
	float dummy = 0.0f;
	float y;
	float ref = 1.0f;
	float error,integral, u = 0.0f;
	struct timespec next, start, stop, sleep;
	long sleep_time_ns = SAMPLE_TIME * S_TO_NS;



	//send start to server
	send_msg(eStart, dummy);
	
	//start measuring time
	clock_gettime(CLOCK_MONOTONIC, &next);

	uint16_t iterations = 0;
	uint16_t numIterations = RUNTIME / SAMPLE_TIME;

	//
	while(iterations < numIterations){

		//take time before sending "GET"
		clock_gettime(CLOCK_MONOTONIC, &start);

		if(iterations == 1000){
			ref = 0.0f;
		}

		//send get
		send_msg(eGet, dummy);

		//wait on receiving the y val from the server
		y = get_item_from_buffer();

		//do calculations 
		
		error = ref - y;
		integral += error * SAMPLE_TIME;
		
		u = Kp * error + Ki * integral;
		
		send_msg(eSet, u);

		//Sleep
		clock_gettime(CLOCK_MONOTONIC, &stop);
		sleep.tv_sec = 0;
		sleep.tv_nsec = sleep_time_ns - (timespec_sub(stop,start).tv_nsec);
		next = timespec_add(stop, sleep);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
		iterations++;
	}

	//send stop to server
	send_msg(eStop, dummy);

	return NULL;

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
	UDPConn* conn = udpconn_new("127.0.0.1", 9999);
    
	char sendBuf[64];
	char recvBuf[64];    
	memset(recvBuf, 0, sizeof(recvBuf));
sprintf(sendBuf, "START");   
	 udpconn_send(conn, sendBuf);
	while(1){

	
	printf("send\r\n");
 	sprintf(sendBuf, "GET");    
	udpconn_send(conn, sendBuf);
	//udpconn_send(conn, sendBuf);
	printf("get: %s\r\n", sendBuf);

	struct timespec next,sleep;
	//start measuring time
	clock_gettime(CLOCK_MONOTONIC, &next);
	printf("time: %lu\r\n", next.tv_nsec);
	sleep.tv_nsec = 1000000;
	sleep.tv_sec = 0;
	//next = timespec_add(next, sleep);
	printf("before sleep: %lu\r\n", next.tv_nsec);
	clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep, NULL);
	printf("after sleep: %lu\r\n", next.tv_nsec);

   	udpconn_receive(conn, recvBuf, sizeof(recvBuf));


	printf(recvBuf);

	/*	

	    sprintf(sendBuf, "GET");    
	    udpconn_send(conn, sendBuf);

	    udpconn_receive(conn, recvBuf, sizeof(recvBuf));
		printf(recvBuf);
	
	sprintf(sendBuf, "STOP");    
	    udpconn_send(conn, sendBuf);
	    
	    udpconn_delete(conn);
	
	*/
	/*sem_init(&semaAck, pShared, value);
	
	pthread_create(&threadController, NULL, thread_controller_func, NULL);
	pthread_create(&threadRecv, NULL, recv_func, NULL);
	pthread_create(&threadSendAck, NULL, send_ack_func,NULL);

	pthread_join(threadController, NULL);
	pthread_join(threadRecv, NULL);
	pthread_join(threadSendAck, NULL);
	*/








	return 0;
}