#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gpio.h>
#include <power_clocks_lib.h>
#include <sysclk.h>
#include <compiler.h>
#include <board.h>
#include <stdio_usb.h>


#define CONFIG_USART_IF (AVR32_USART2)

#include "FreeRTOS.h"
#include "task.h"

#define TEST_A      AVR32_PIN_PA31
#define RESPONSE_A  AVR32_PIN_PA30
#define TEST_B      AVR32_PIN_PA29
#define RESPONSE_B  AVR32_PIN_PA28
#define TEST_C      AVR32_PIN_PA27
#define RESPONSE_C  AVR32_PIN_PB00


void busy_delay_ms(int delay){
    for(; delay != 0; delay--){
        for(int i = 0; i < 2108; i++){
            asm volatile ("" ::: "memory");
        }
    }
}

void busy_delay_short(void){
    for(int i = 0; i < 10; i++){
        asm volatile ("" ::: "memory");
    }
}

void init(){
	sysclk_init();
	
	board_init();
	
    gpio_configure_pin(TEST_A, GPIO_DIR_INPUT);
    gpio_configure_pin(TEST_B, GPIO_DIR_INPUT);
    gpio_configure_pin(TEST_C, GPIO_DIR_INPUT);
    gpio_configure_pin(RESPONSE_A, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
    gpio_configure_pin(RESPONSE_B, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
    gpio_configure_pin(RESPONSE_C, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);

	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
	
	stdio_usb_init(&CONFIG_USART_IF);

    #if defined(__GNUC__) && defined(__AVR32__)
	    setbuf(stdout, NULL);
	    setbuf(stdin,  NULL);
    #endif
}

/* Task A
static void taskFn1(void* args){
	const portTickType delay = 200 / portTICK_RATE_MS;
	
    int iter = 0;

	while(1){
		gpio_toggle_pin(LED0_GPIO);	
		vTaskDelay(delay);
	}
}

static void taskFn2(void* args){
	const portTickType delay = 500 / portTICK_RATE_MS;
	
	int iter = 0;

	while(1){
		gpio_toggle_pin(LED1_GPIO);
		vTaskDelay(delay);
	}
}


int main(){
	init();
    
	xTaskCreate(taskFn1, "task1", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(taskFn2, "task2", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
	// Start the scheduler, anything after this will not run.
	
	vTaskStartScheduler();
    
}

*/

struct responseTaskArgs {
	struct {
		uint32_t test;
		uint32_t response;
	} pin;
	
	// other args ...
};
/*static void responseTask(void* args){
	struct responseTaskArgs a = *(struct responseTaskArgs*)args;
	while(1){
		if(gpio_pin_is_low(a.pin.test)){
			gpio_set_pin_low(a.pin.response);
			//vTaskDelay(1);
			busy_delay_short();
			gpio_set_pin_high(a.pin.response);
		}
	}
}

static void responseTaskC(void* args){
	struct responseTaskArgs a = *(struct responseTaskArgs*)args;
	while(1){
		if(gpio_pin_is_low(a.pin.test)){
			busy_delay_ms(3);
			gpio_set_pin_low(a.pin.response);
			//vTaskDelay(1);	
			busy_delay_short();
			gpio_set_pin_high(a.pin.response);
		}
	}
}*/

static void responseTask(void* args){
	struct responseTaskArgs a = *(struct responseTaskArgs*)args;
	while(1){
		if(!gpio_pin_is_low(a.pin.test)){
			vTaskDelay(1);
		}
		else{
			busy_delay_ms(1);
			
			gpio_set_pin_low(a.pin.response);
			//vTaskDelay(1);
			busy_delay_short();
			gpio_set_pin_high(a.pin.response);
		}
		
	}
}

static void responseTaskC(void* args){
	struct responseTaskArgs a = *(struct responseTaskArgs*)args;
	while(1){
		if(!gpio_pin_is_low(a.pin.test)){
			vTaskDelay(1);
		}
		else{
			busy_delay_ms(3);
			
			gpio_set_pin_low(a.pin.response);
			//vTaskDelay(1);
			busy_delay_short();
			gpio_set_pin_high(a.pin.response);
		}
	}
	
}


int main(){
	init();
	
	xTaskCreate(responseTask, "TEST_A", 1024,	(&(struct responseTaskArgs){{TEST_A, RESPONSE_A}}),	tskIDLE_PRIORITY + 3, NULL);
	xTaskCreate(responseTask, "TEST_B", 1024,	(&(struct responseTaskArgs){{TEST_B, RESPONSE_B}}),	tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(responseTaskC, "TEST_C", 1024,	(&(struct responseTaskArgs){{TEST_C, RESPONSE_C}}),	tskIDLE_PRIORITY + 1, NULL);
		
	vTaskStartScheduler();
}
