#include "communication.h"
#include "udp.h"
#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

static pthread_mutex_t udp_mtx;

static char* strMessages[] = {"START", "GET", "SET", "STOP", "SIGNAL_ACK"};

//shared resource for buffer cnt 
static sem_t semaCntYVal;


static pthread_mutex_t mtxBuffer;
static ring_buf_t* ringBuf;


static UDPConn* conn;

// ---------------------------------------------
// initializes udp communication
// ---------------------------------------------
void init_communication(){
	//set up udp socket
	conn = udpconn_new("127.0.0.1", 9999);
    

	//initialization of ringBuf
	ringBuf = malloc(sizeof(ring_buf_t));
	ringBuf->head = 0;
	ringBuf->tail = 0;

	sem_init(&semaCntYVal, 0, 0);


    //udpconn_delete(conn);

}

void send_msg(enMessage msg, float val){
	char sendBuf[64];
	//add val to message for setting the u value
	if(msg == eSet){
		sprintf(sendBuf, "%s:%3.3f",strMessages[msg], val);    

	}
	else{
		sprintf(sendBuf, "%s",strMessages[msg]);
	}    

	udpconn_send(conn, sendBuf);
}


void receive_msg(char* recvBuf){
	   
   udpconn_receive(conn, recvBuf, sizeof(recvBuf));

}


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
	

}