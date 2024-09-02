
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#define SYS_CLK 16000000
#define SYS_PERIOD 62
#include "BSP.h"
#include "miros.h"



 OSthread green;
 OSthread red;
uint32_t stack_blinky1[40] __attribute__ ((aligned (8)));
uint32_t *sp_blinky1 = &stack_blinky1[40];

uint32_t stack_blinky2[40] __attribute__ ((aligned (8)));
uint32_t *sp_blinky2 = &stack_blinky2[40];
void main_blinkred(void) {
	while(1){
		OS_delay(15);
		RedLED_ON();
		OS_delay(15);
		RedLED_OFF();

	}


}
void main_blinkgreen(void) {


	while(1){

		OS_delay(30);
		GreenLED_ON();
		OS_delay(30);
		GreenLED_OFF();

	}


}

void idle_thread_f(){
	while(1);
}




int main(void) {

 init_LEDS(); //Configures I/O ports connected to the LEDs
 OS_init();
 OSthread_Sched( &green,  &main_blinkgreen,stack_blinky1,sizeof(stack_blinky1));
 OSthread_Sched( &red,  &main_blinkred,stack_blinky2,sizeof(stack_blinky2));
 OS_run();   //Start OS after all processes have been created to avoid any context switching in the middle of process creation
 	 	 	 // this includes configuring the systick timer enabling its interrupt and setting its priority to HIGHEST
 	 	 	 //enabling pendsv interrupt and setting its priority to LOWEST
 	 	 	 //scheduling first process to run

 /*CONTROL SHOULD NEVER RETURN HERE SO WE PLACE AN ASSERTION*/
 while(1);

}
