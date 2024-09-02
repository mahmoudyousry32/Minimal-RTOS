#define RED_LED		GPIO_PIN_14
#define GREEN_LED		GPIO_PIN_13
#define CYCLES_PER_MSEC 1000

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "bsp.h"
#include "miros.h"
static uint32_t volatile l_tickCtr = 0;

void init_systick(){
	//SystemCoreClockUpdate();
  SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC); //trigger each 1 ms
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_SetPriority(SysTick_IRQn, 0);

;


}
void init_pendsv(){
	NVIC_EnableIRQ(PendSV_IRQn);
	NVIC_SetPriority(PendSV_IRQn, 15);
}

void GreenLED_ON(){
	GPIOG->BSRR = GPIO_BSRR_BS13 ;

}

void GreenLED_OFF(){
	GPIOG->BSRR = GPIO_BSRR_BR13 ;

}



void RedLED_ON(){
	GPIOG->BSRR = GPIO_BSRR_BS14 ;

}

void RedLED_OFF(){
	GPIOG->BSRR = GPIO_BSRR_BR14;

}
void init_LEDS(){
	__HAL_RCC_GPIOG_CLK_ENABLE();
	GPIO_InitTypeDef LEDS;
	LEDS.Mode = GPIO_MODE_OUTPUT_PP;
	LEDS.Pin = GPIO_PIN_13 | GPIO_PIN_14;
	HAL_GPIO_Init(GPIOG,&LEDS);

}

void delay_S(uint32_t timeSec){
	for(uint32_t i = 0; i <4000000*timeSec ; i++);
}

void delay_ms(uint32_t timeMs){
	for(uint32_t i = 0; i <4000*timeMs ; i++);
}



void SysTick_Handler(void) {
	__disable_irq();
    ++l_tickCtr; //incrememnts each 1 ms
    OS_tick();
	OS_Sched();
	__enable_irq();
}

uint32_t BSP_tickCtr(void) {
    uint32_t tickCtr;

    //__disable_irq();
    tickCtr = l_tickCtr;
   // __enable_irq();

    return tickCtr;
}

void BSP_delay(uint32_t ticks) {
    uint32_t start = BSP_tickCtr();
    while ((BSP_tickCtr() - start) < ticks) { //gets the value of l_tickctr (ms counter) it essentially is just a way to create ms delay
    }
}

void OS_startup(){

	init_systick();
	init_pendsv();

}
