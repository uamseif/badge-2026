#include "gpio_hal.h"
#include "string.h"

static uint8_t halKeySavedKeys; /* Keep the last state of the button to query whether there is a key value change */

static pHalKeyProcessFunction _pHalKeyProcessFunction; /* callback function */

static uint16_t halKeyState = 0;
static uint16_t halKeyLast = 0;

static uint8_t halKeyCounter[5] = {0};

void HAL_KeyInit(void)
{
    halKeySavedKeys = 0;

    halKeyLast = 0;
    memset(halKeyCounter, 0, sizeof(halKeyCounter));

    _pHalKeyProcessFunction = NULL;

    RCC_APB2PeriphClockCmd(
            RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC,
            ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;

    // GPIOD keys
    GPIO_InitStructure.GPIO_Pin =
            KEY1_BV |
            KEY2_BV |
            KEY4_BV |
            KEY5_BV;

    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // GPIOC key
    GPIO_InitStructure.GPIO_Pin = KEY3_BV;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void HalKeyConfig(pHalKeyProcessFunction cback) {
    _pHalKeyProcessFunction = cback;
}

uint8_t HalKeyRead(void) {
    uint16_t keys = 0;

    if(HAL_PUSH_BUTTON1()) {
        keys |= HAL_KEY_SW_MENU;
    } if(HAL_PUSH_BUTTON2()) {
        keys |= HAL_KEY_SW_B;
    } if(HAL_PUSH_BUTTON3()) {
        keys |= HAL_KEY_SW_C;
    } if(HAL_PUSH_BUTTON4()) {
        keys |= HAL_KEY_SW_A;
    } if(HAL_PUSH_BUTTON5()) {
        keys |= HAL_KEY_SW_D;
    }
    return keys;
}

void HAL_KeyPoll(void) {
    uint16_t keys = HalKeyRead();
    uint16_t changed = keys ^ halKeyLast;

    for(uint8_t i = 0; i < 5; i++)
    {
        uint16_t mask = (1 << i);

        if(keys & mask)
        {
            if(halKeyCounter[i] < 255)
                halKeyCounter[i]++;

            if(halKeyCounter[i] == HAL_KEY_LONG_TIME)
            {
                /* Initial long press */
                if(_pHalKeyProcessFunction)
                    _pHalKeyProcessFunction(mask, HAL_KEY_EVENT_LONG);
            }
            else if(halKeyCounter[i] > HAL_KEY_LONG_TIME &&
                    ((halKeyCounter[i] - HAL_KEY_LONG_TIME) % HAL_KEY_REPEAT_TIME) == 0)
            {
                /* Auto-repeat: fire LONG every REPEAT_TIME polls */
                if(_pHalKeyProcessFunction)
                    _pHalKeyProcessFunction(mask, HAL_KEY_EVENT_LONG);
            }
        }
        else
        {
            /* Fire UP on release regardless of press duration */
            if(halKeyCounter[i] >= HAL_KEY_DEBOUNCE_TIME)
            {
                if(_pHalKeyProcessFunction)
                    _pHalKeyProcessFunction(mask, HAL_KEY_EVENT_UP);
            }

            halKeyCounter[i] = 0;
        }

        if(changed & mask)
        {
            if(keys & mask)
            {
                if(_pHalKeyProcessFunction)
                    _pHalKeyProcessFunction(mask, HAL_KEY_EVENT_DOWN);
            }
        }
    }

    halKeyLast = keys;
}