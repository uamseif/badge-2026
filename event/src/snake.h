#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

#define SNAKE_W      16   /* playfield width  (horizontal axis, rows 0-15) */
#define SNAKE_H       8   /* playfield height (vertical axis,  cols 0-7)   */
#define SNAKE_MAXLEN 32   /* max snake body segments                        */

/*
 * Button layout — horizontal orientation:
 *
 *  B (bottom-left) = turn left
 *  C (bottom-right) = turn right
 *  Menu             = exit to menu
 *
 * Directions: 0=RIGHT (+row), 1=DOWN (+col), 2=LEFT (-row), 3=UP (-col)
 */

typedef struct {
    int8_t   body_r[SNAKE_MAXLEN]; /* row of each segment (head = [0]) */
    int8_t   body_c[SNAKE_MAXLEN]; /* col of each segment              */
    uint8_t  len;
    uint8_t  dir;                  /* 0=R 1=D 2=L 3=U                  */
    int8_t   food_r, food_c;
    uint16_t score;
    bool     game_over;
    uint32_t last_tick;
    uint32_t step_ms;
} Snake;

void snake_init(Snake *s);
/* Returns true when the player exits to the menu. */
bool snake_update(Snake *s, CBTS_MATRIX *display);

#endif /* SNAKE_H */
