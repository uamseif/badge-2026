#if 0
#ifndef ARKANOID_H
#define ARKANOID_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

#define ARK_W          8   /* playfield width  (cols 0-7) */
#define ARK_H         16   /* playfield height (rows 0-15) */
#define ARK_BRICK_ROWS 6   /* rows 0-5 are bricks */
#define ARK_PADDLE_W   3   /* paddle pixel width */

typedef struct {
    uint8_t  bricks[ARK_BRICK_ROWS];
    int8_t   paddle_x;
    int8_t   ball_r, ball_c;
    int8_t   bdr, bdc;
    uint8_t  lives;
    uint8_t  level;
    uint16_t score;
    uint32_t ball_ms;
    bool     serving;
    bool     game_over;
    uint32_t ball_last_tick;
    uint32_t paddle_last_tick;
} Arkanoid;

void arkanoid_init(Arkanoid *a);
bool arkanoid_update(Arkanoid *a, CBTS_MATRIX *display);

#endif /* ARKANOID_H */
#endif
