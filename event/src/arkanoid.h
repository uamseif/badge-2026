#ifndef ARKANOID_H
#define ARKANOID_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

#define ARK_W          8   /* playfield width  (cols 0-7) */
#define ARK_H         16   /* playfield height (rows 0-15) */
#define ARK_BRICK_ROWS 6   /* rows 0-5 are bricks */
#define ARK_PADDLE_W   3   /* paddle pixel width */

/*
 * Button layout — vertical orientation:
 *
 *  C (bottom-right) = move paddle left  (col++)
 *  D (top-right)    = move paddle right (col--)
 *  Menu             = exit to menu
 *  A / B            = launch ball while serving
 *
 * Col 0 = visual right, col 7 = visual left (inverted, same as Tetris).
 */

typedef struct {
    uint8_t  bricks[ARK_BRICK_ROWS]; /* bitmask: bit c = col c alive */
    int8_t   paddle_x;               /* leftmost col of paddle (0 to ARK_W-ARK_PADDLE_W) */
    int8_t   ball_r, ball_c;
    int8_t   bdr, bdc;               /* ball row/col velocity (+1 or -1) */
    uint8_t  lives;
    uint8_t  level;
    uint16_t score;
    uint32_t ball_ms;                /* ms per ball step */
    bool     serving;
    bool     game_over;
    uint32_t ball_last_tick;
    uint32_t paddle_last_tick;
} Arkanoid;

void arkanoid_init(Arkanoid *a);
/* Returns true when the player exits to the menu. */
bool arkanoid_update(Arkanoid *a, CBTS_MATRIX *display);

#endif /* ARKANOID_H */
