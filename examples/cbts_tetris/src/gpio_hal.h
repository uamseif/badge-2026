#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include "stdint.h"
#include "ch32v00x.h"
#include <stddef.h>


#define HAL_KEY_POLLING_VALUE    100
#define BV(n)      (1 << (n))
/* Switches (keys) */
#define HAL_KEY_SW_1             0x01  // key1
#define HAL_KEY_SW_2             0x02  // key2
#define HAL_KEY_SW_3             0x04  // key3
#define HAL_KEY_SW_4             0x08  // key4
#define HAL_KEY_SW_5             0x10  // key5

#define KEY1_PCENR               (RCC_APB2Periph_GPIOD)
#define KEY2_PCENR               (RCC_APB2Periph_GPIOD)
#define KEY3_PCENR               (RCC_APB2Periph_GPIOC)
#define KEY4_PCENR               (RCC_APB2Periph_GPIOD)
#define KEY5_PCENR               (RCC_APB2Periph_GPIOD)

#define KEY1_GPIO                (GPIOD)
#define KEY2_GPIO                (GPIOD)
#define KEY3_GPIO                (GPIOC)
#define KEY4_GPIO                (GPIOD)
#define KEY5_GPIO                (GPIOD)

#define KEY1_BV                  GPIO_Pin_4
#define KEY2_BV                  GPIO_Pin_0
#define KEY3_BV                  GPIO_Pin_0
#define KEY4_BV                  GPIO_Pin_3
#define KEY5_BV                  GPIO_Pin_2

#define KEY1_IN                  (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_BV)==0)
#define KEY2_IN                  (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_BV)==0)
#define KEY3_IN                  (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_BV)==0)
#define KEY4_IN                  (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_BV)==0)
#define KEY5_IN                  (GPIO_ReadInputDataBit(KEY5_GPIO, KEY5_BV)==0)

#define HAL_PUSH_BUTTON1()       (KEY1_IN) //Add custom button
#define HAL_PUSH_BUTTON2()       (KEY2_IN)
#define HAL_PUSH_BUTTON3()       (KEY3_IN)
#define HAL_PUSH_BUTTON4()       (KEY4_IN)
#define HAL_PUSH_BUTTON5()       (KEY5_IN)

#define HAL_KEY_DEBOUNCE_TIME     2
#define HAL_KEY_LONG_TIME         100

#define HAL_KEY_EVENT_DOWN      0x01
#define HAL_KEY_EVENT_UP        0x02
#define HAL_KEY_EVENT_LONG      0x04

typedef void (*pHalKeyProcessFunction)(uint16_t key, uint8_t state);

typedef struct
{
    uint8_t keys; // keys
} keyChange_t;


void HAL_KeyInit(void);

void HAL_KeyPoll(void);

void Hal_KeyConfig(const pHalKeyProcessFunction cback);

uint8_t Hal_KeyRead(void);

#endif //GPIO_HAL_H
