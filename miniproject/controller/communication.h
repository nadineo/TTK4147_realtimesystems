#ifndef COMMUNICATION_H
#define COMMUNICATION_H

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




void init_communication();
void send_msg(enMessage msg, float val);
void receive_msg(char* recvBuf);
void put_item_in_buffer(float val);
float get_item_from_buffer();



#endif