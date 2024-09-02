/*
 * BSP.h
 *
 *  Created on: Aug 29, 2024
 *      Author: pc
 */

#ifndef EXAMPLE_USER_BSP_H_
#define EXAMPLE_USER_BSP_H_
#define BSP_TICKS_PER_SEC 100U

#include <stdint.h>
void init_systick();
void GreenLED_ON();
void GreenLED_OFF();
void RedLED_ON();
void RedLED_OFF();
void init_LEDS();
void delay_S(uint32_t timeSec);
void delay_ms(uint32_t timeMs);
void SysTick_Handler(void);
uint32_t BSP_tickCtr(void);
void BSP_delay(uint32_t ticks);



#endif /* EXAMPLE_USER_BSP_H_ */
