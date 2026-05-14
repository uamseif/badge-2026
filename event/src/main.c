#include <ch32v00x.h>
#include "i2c_hal.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include "cbts_matrix.h"
#include "menu.h"
#include "stats.h"
#include "tetris.h"
#include "pong.h"
#include "snake.h"
/* #include "arkanoid.h" */
#include "flappy.h"
#include "space_inv.h"
#include "frogger.h"
#include "life.h"
#include "name_editor.h"

void Delay_Init(void);

#define I2C_SPEED_400KHZ   400000
#define I2C_MASTER_ADDRESS 0x00

static CBTS_MATRIX display;
static union {
    Tetris        t;
    Pong          p;
    Snake         s;
    /* Arkanoid   a; */
    Flappy        fl;
    SpaceInvaders si;
    Frogger       fr;
    Life          li;
    NameEditor    ne;
} game;

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

    uint8_t run = APP_MARQUEE;

    while (1) {
        HAL_KeyPoll();   /* fires registered callbacks */

        switch (run) {
            case APP_MARQUEE:
                if (marquee_update(&display)) {
                    menu_init();
                    game_state = run = APP_MENU;
                }
                break;

            case APP_MENU: {
                uint8_t sel = (uint8_t)menu_update(&display);
                if (sel == APP_MARQUEE)     marquee_init();
                if (sel == APP_TETRIS)      tetris_init(&game.t);
                if (sel == APP_PONG)        pong_init(&game.p);
                if (sel == APP_SNAKE)       snake_init(&game.s);
                /* if (sel == APP_ARKANOID) arkanoid_init(&game.a); */
                if (sel == APP_FLAPPY)      flappy_init(&game.fl);
                if (sel == APP_SINVADERS)   si_init(&game.si);
                if (sel == APP_FROGGER)     frogger_init(&game.fr);
                if (sel == APP_NAME_EDITOR) name_editor_init(&game.ne);
                game_state = run = sel;
                break;
            }

            case APP_TETRIS:
                if (tetris_update(&game.t, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break;

            case APP_PONG:
                if (pong_update(&game.p, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break;

            case APP_SNAKE:
                if (snake_update(&game.s, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break;

            /* case APP_ARKANOID:
                if (arkanoid_update(&game.a, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break; */

            case APP_FLAPPY:
                if (flappy_update(&game.fl, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break;

            case APP_SINVADERS:
                if (si_update(&game.si, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break;

            case APP_FROGGER:
                if (frogger_update(&game.fr, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break;

            case APP_LIFE:
                if (life_update(&game.li, &display)) {
                    menu_init(); game_state = run = APP_MENU;
                }
                break;

            case APP_NAME_EDITOR:
                if (name_editor_update(&game.ne, &display)) {
                    if (game_state == (uint8_t)APP_NAME_EDITOR) {
                        game_state = APP_MENU;
                        menu_init();
                        run = APP_MENU;
                    } else {
                        run = game_state;
                    }
                }
                break;

            default:
                menu_init(); game_state = APP_MENU; run = APP_MENU;
                break;
        }

        HAL_delay_1ms(16);
    }
}
