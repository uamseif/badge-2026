#include <stdio.h>
#include <ch32v00x.h>
#include "cbts_matrix.h"
#include "i2c_hal.h"
#include "gpio_hal.h"
#include "tetris.h"

void Delay_Init(void);
void Delay_Ms(uint32_t n);

#define I2C_SPEED_400KHZ 400000
#define I2C_MASTER_MY_ADDRESS 0x00

CBTS_MATRIX display;

void key_handler(uint16_t key, uint8_t state) {
    if(key == HAL_KEY_SW_1) {
        if(state == HAL_KEY_EVENT_DOWN) {
            //printf("KEY1 DOWN\n");
        }

        if(state == HAL_KEY_EVENT_UP) {
            //printf("KEY1 SHORT\n");

        }

        if(state == HAL_KEY_EVENT_LONG) {
            //printf("KEY1 LONG\n");
        }
    }
}

uint8_t tetris_framebuffer[16] = {0};

int main() {

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    HAL_Systick_Init();
    HAL_I2C_Init(I2C_SPEED_400KHZ, I2C_MASTER_MY_ADDRESS);
    HAL_KeyInit();
    Hal_KeyConfig(TETRIS_key_callback);


    CBTS_MATRIX_init(&display);
    CBTS_MATRIX_begin(&display);

    TETRIS_setup();

    while (1) {
        HAL_KeyPoll();
        TETRIS_loop(tetris_framebuffer);

        for (uint8_t y = 0; y < GRID_H; y++) {
            for (uint8_t x = 0; x < GRID_W; x++) {
                CBTS_MATRIX_setLed(&display, RED_DRIVER, y, x, (tetris_framebuffer[y] >> x) & 0x01);

            }
            tetris_framebuffer[y] = 0;
        }

        CBTS_MATRIX_show(&display);
        HAL_delay_1ms(50);
    }
}
