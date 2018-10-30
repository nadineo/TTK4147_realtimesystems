
#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "udp.h"
#include "time.h"
#include <semaphore.h>


#define Kp 10
#define Ki 800
#define SAMPLE_TIME 0.001
#define RUNTIME 2
#define S_TO_NS 1000000000

#define BUF_LEN 128
//enum for sending messages to server
typedef enum enMessage
{
	eStart = 0,
	eGet = 1,
	eSet = 2,
	eStop = 3,
	eAck = 4

};

typedef struct ring_buf_t
{
	float buffer[BUF_LEN];
	uint8_t head;
	uint8_t tail;
}ring_buf_t;


//receive thread -> handles incoming messages
static pthread_t threadRecv;

//thread that sends the acknowledge to server
static pthread_t threadSendAck;

//thread that performs the controller logic
static pthread_t threadController;


//shared resource for acknowledge -> semaphore
static sem_t semaAck;

//shared resource for buffer cnt 
static sem_t semaCntYVal;


static pthread_mutex_t mtxBuffer;
static pthread_mutex_t mtxThreadSync;
static int controlActive;
static ring_buf_t* ringBuf;

static UDPConn* conn;

static const char* STR_SIGNAL = "SIGNAL";
static const char* STR_GET_ACK = "GET_ACK:";



void put_item_in_buffer(float val){
	
	//store y val in buffer
	pthread_mutex_lock(&mtxBuffer);

	ringBuf->buffer[ringBuf->tail] = val;
	ringBuf->tail = (ringBuf->tail +1)%BUF_LEN;

	//unlock buffer
	pthread_mutex_unlock(&mtxBuffer);
	//post semaphore
	sem_post(&semaCntYVal);

}

float get_item_from_buffer(){
	float val;
	sem_wait(&semaCntYVal);
	//lock buffer
	pthread_mutex_lock(&mtxBuffer);
	val = ringBuf->buffer[ringBuf->head];
	ringBuf->head = (ringBuf->head+1)%BUF_LEN;
	//unlock buffer
	pthread_mutex_unlock(&mtxBuffer);
	
	return val;

}


// ------------------------------------------------
// thread function for receiving messages 
// ------------------------------------------------
void* recv_func(void* param){
	char recvBuf[64];
	memset(recvBuf, 0, sizeof(recvBuf));
	char * pch;
	char * temp;
	float y = 0.0f;
	int lastActive = 0;

    while(1){
    	if(get_control_active()){
    		lastActive = 1;
			udpconn_receive(conn, recvBuf, sizeof(recvBuf));

			//check if SIGNAL is sent
			if(strcmp(recvBuf, STR_SIGNAL) == 0){
				//increment semaphore
				sem_post(&semaAck);
			}
			else if(strcmp(recvBuf, STR_GET_ACK) == 0){
				
				pch = strtok(recvBuf,":");
					
			    while (pch != NULL)
			    {

			        temp = pch;
			        pch = strtok (NULL, ": ");
			    }
			    y = atof(temp);

				//put item in buffer
				put_item_in_buffer(y);

			}
		}
		else if(lastActive){
			break;
		}
	}
    return NULL;
}

static void control(timespec & req, timespec & ref, timespec & period){
	struct timespec old_time, wakeup_time;

	while(timespec_cmp(req, ref) > 0){
	    
	    
        sprintf(sendBuf, "GET");    
        udpconn_send(conn, sendBuf);

        clock_gettime(CLOCK_MONOTONIC, &old_time);
       
		//wait on receiving the y val from the server
		y = get_item_from_buffer();
		
		//udpconn_receive(conn, recvBuf, sizeof(recvBuf));

		//do calculations 
		error = ref - y;
		integral += error * SAMPLE_TIME;
		
		u = Kp * error + Ki * integral;
		
		//send calculated u value
        sprintf(sendBuf, "SET:%d", u);
        udpconn_send(conn, sendBuf); 

        //calculate wakeup time -> add period to measured time after "GET" command 
        wakeup_time = timespec_add(old_time, period);
		//Sleep the remaining period
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
		
		//update curr time
		clock_gettime(CLOCK_MONOTONIC, &ref);

		
	}
	

}

void update_control_active(int isActive){
	pthread_mutex_lock(mtxThreadSync);
	controlActive = isActive;
	pthread_mutex_lock(mtxThreadSync);
}

int get_control_active(){
	int isActive;
	pthread_mutex_lock(mtxThreadSync);
	isActive = controlActive;
	pthread_mutex_lock(mtxThreadSync);
	return isActive;
}


void* controller_func(void * param){
	float ref = 1.0f;
	float error,integral, y, u = 0.0f;
	char sendBuf[64];
	char recvBuf[64];
	memset(recvBuf, 0, sizeof(recvBuf));

	
	struct timespec half_control_time = timespec_normalized(1,0);
    struct timespec end_control_time = timespec_normalized(2,0);
    struct timespec period = timespec_normalized(0,1*1000*1000);
	struct timespec new_time, curr_time, sleep_time;
	
	clock_gettime(CLOCK_MONOTONIC, &curr_time);
	
	struct timespec half_time = timespec_add(curr_time,half_control_time);
	struct timespec stop_time = timespec_add(curr_time, end_control_time);
	
	//START simulation
	sprintf(sendBuf, "START");
	udpconn_send(conn, sendBuf);

	update_control_active(1);
	
    
    //set reference
    sprintf(sendBuf, "SET:%d", ref);
    udpconn_send(conn, sendBuf); 
    printf("set is done");
    
    // --------------------------------------------------------------------
    // start with the first half of the simulation -> reference value = 1
    // --------------------------------------------------------------------
    control(half_time, curr_time, period);


	// --------------------------------------------------------------------
    // second half of the simulation -> reference value = 0
    // --------------------------------------------------------------------
    ref = 0.0f;
    sprintf(sendBuf, "SET:%3.3f", ref);
    udpconn_send(conn, sendBuf); 

    control(stop_time, curr_time, period);

	//stop simulation
    sprintf(sendBuf, "STOP");    
    udpconn_send(conn, sendBuf);

    update_control_active(0);
 

	return NULL;

}
// ------------------------------------------------
// thread function for sending acknowledge
// ------------------------------------------------
void* send_ack_func(void* param){
	float dummy = 0.0f;
    char sendBuf[64];
    int lastActive = 0;
    
	while(1){
		if(get_control_active()){
			sem_wait(&semaAck);
			sprintf(sendBuf, "SIGNAL_ACK");    
	        udpconn_send(conn, sendBuf);
	        lastActive = 1;
	    }
	    else if(lastActive){
	    	break;
	    }
	}

	return NULL;
    
}


int main(){
	conn = udpconn_new("192.168.0.1", 9999);


    sem_init(&semaAck, 0, 0);

	//initialization of ringBuf
	ringBuf = malloc(sizeof(ring_buf_t));
	ringBuf->head = 0;
	ringBuf->tail = 0;

	sem_init(&semaCntYVal, 0, 0);
	update_control_active(0);
	
	pthread_create(&threadController, NULL, controller_func, NULL);
	pthread_create(&threadRecv, NULL, recv_func, NULL);
	pthread_create(&threadSendAck, NULL, send_ack_func,NULL);

	pthread_join(threadController, NULL);
	pthread_join(threadRecv, NULL);
	pthread_join(threadSendAck, NULL);
	
    udpconn_delete(conn);
	return 0;
	
	
	


}
