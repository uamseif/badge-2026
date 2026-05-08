/*
 * systick_hal.h
 *
 *  Created on: Sep 27, 2023
 *      Author: Sergo
 */

#ifndef SYSTICK_HAL_H_
#define SYSTICK_HAL_H_

#include "ch32v00x.h"

void HAL_Systick_Init (void);

void HAL_Systick_Increment (void);

void HAL_delay_1ms(uint32_t count);

void HAL_delay_decrement(void);

uint32_t HAL_get_tick (void);

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

#endif /* SYSTICK_HAL_H_ */
