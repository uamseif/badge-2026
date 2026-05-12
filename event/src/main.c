#include <ch32v00x.h>
#include "i2c_hal.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include "cbts_matrix.h"
#include "menu.h"
#include "tetris.h"
#include "pong.h"
#include "snake.h"
#include "arkanoid.h"
#include "flappy.h"
#include "space_inv.h"
#include "frogger.h"
#include "life.h"

void Delay_Init(void);

#define I2C_SPEED_400KHZ   400000
#define I2C_MASTER_ADDRESS 0x00

static CBTS_MATRIX display;
static union {
    Tetris   t;
    Pong     p;
    Snake    s;
    Arkanoid a;
    Flappy        fl;
    SpaceInvaders si;
    Frogger       fr;
    Life          li;
} game;
static AppState state = APP_MARQUEE;

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
                if (state == APP_MARQUEE)  marquee_init();
                if (state == APP_TETRIS)   tetris_init(&game.t);
                if (state == APP_PONG)     pong_init(&game.p);
                if (state == APP_SNAKE)    snake_init(&game.s);
                if (state == APP_ARKANOID) arkanoid_init(&game.a);
                if (state == APP_FLAPPY)     flappy_init(&game.fl);
                if (state == APP_SINVADERS)  si_init(&game.si);
                if (state == APP_FROGGER)    frogger_init(&game.fr);
                if (state == APP_LIFE)       life_init(&game.li);
                break;

            case APP_TETRIS:
                if (tetris_update(&game.t, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_PONG:
                if (pong_update(&game.p, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_SNAKE:
                if (snake_update(&game.s, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_ARKANOID:
                if (arkanoid_update(&game.a, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_FLAPPY:
                if (flappy_update(&game.fl, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_SINVADERS:
                if (si_update(&game.si, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_FROGGER:
                if (frogger_update(&game.fr, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;

            case APP_LIFE:
                if (life_update(&game.li, &display)) {
                    menu_init();
                    state = APP_MENU;
                }
                break;
        }

        HAL_delay_1ms(16);
    }
}
