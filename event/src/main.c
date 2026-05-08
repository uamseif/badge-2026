#include <ch32v00x.h>
#include "i2c_hal.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include "cbts_matrix.h"
#include "menu.h"
#include "tetris.h"
#include "pong.h"
#include "snake.h"

void Delay_Init(void);

#define I2C_SPEED_400KHZ   400000
#define I2C_MASTER_ADDRESS 0x00

static CBTS_MATRIX display;
static Tetris      tetris_game;
static Pong        pong_game;
static Snake       snake_game;
static AppState    state = APP_MARQUEE;

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    HAL_Systick_Init();
    HAL_I2C_Init(I2C_SPEED_400KHZ, I2C_MASTER_ADDRESS);
    HAL_KeyInit();

    CBTS_MATRIX_init(&display);
    CBTS_MATRIX_begin(&display);

    marquee_init();

    while (1) {
        HAL_KeyPoll();   /* fires registered callbacks */

        switch (state) {
            case APP_MARQUEE:
                if (marquee_update(&display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_MENU:
                state = menu_update(&display);
                if (state == APP_TETRIS) tetris_init(&tetris_game);
                if (state == APP_PONG)   pong_init(&pong_game);
                if (state == APP_SNAKE)  snake_init(&snake_game);
                break;

            case APP_TETRIS:
                if (tetris_update(&tetris_game, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_PONG:
                if (pong_update(&pong_game, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_SNAKE:
                if (snake_update(&snake_game, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;
        }

        HAL_delay_1ms(16);
    }
}
