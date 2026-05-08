/*
 * systick_hal.c
 *
 *  Created on: Sep 27, 2023
 *      Author: Sergo
 */
#include "systick_hal.h"

volatile static uint32_t delay;
static uint32_t ticks;

void HAL_Systick_Init (void)
{
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->SR &= ~(1 << 0);
    SysTick->CMP = SystemCoreClock/1000;
    SysTick->CNT = 0;
    SysTick->CTLR = 0xF;
}

void HAL_Systick_Increment (void)
{
    ticks++;
}

void HAL_delay_1ms(uint32_t count)
{
    delay = count;

    while(0U != delay){
    }
}

void HAL_delay_decrement(void)
{
    if (0U != delay){
        delay--;
    }
}

uint32_t HAL_get_tick (void)
{
    return ticks;
}

/*********************************************************************
 * @fn      SysTick_Handler
 *
 * @brief   SysTick_Handler Service Function.
 *
 * @return  none
 */
void SysTick_Handler(void)
{
    SysTick->SR = 0;
    HAL_Systick_Increment();
    HAL_delay_decrement();
}
