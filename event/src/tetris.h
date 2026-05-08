#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"
#include "gpio_hal.h"

#define TETRIS_W        8
#define TETRIS_H        16
#define TETRIS_PIECES   7

/*
 * Button layout — vertical (Tetris) orientation:
 *
 *  top row:    [ B (rotate) ]  [ A (soft drop) ]
 *  bottom row: [ C (left)   ]  [ D (right)     ]
 *  side:         Menu = hard drop / exit to menu on game over
 */
#define BTN_LEFT        HAL_KEY_SW_C
#define BTN_RIGHT       HAL_KEY_SW_D
#define BTN_ROTATE      HAL_KEY_SW_B
#define BTN_SOFT_DROP   HAL_KEY_SW_A
#define BTN_HARD_DROP   HAL_KEY_SW_MENU

typedef struct {
    uint8_t  board[TETRIS_H][TETRIS_W]; // 0=empty, 1-7=LedColor+1
    int8_t   piece;     // current tetromino index 0-6
    int8_t   rot;       // current rotation 0-3
    int8_t   px, py;    // piece origin in board coords
    int8_t   next;      // next tetromino index
    uint16_t score;
    uint16_t lines;
    uint8_t  level;
    bool     game_over;
    uint32_t last_fall; // tick of last gravity step
    uint32_t fall_ms;   // ms between gravity steps
} Tetris;

void tetris_init(Tetris *t);
/* Returns true when the player exits to the menu (MENU button on game over). */
bool tetris_update(Tetris *t, CBTS_MATRIX *display);

#endif /* TETRIS_H */
