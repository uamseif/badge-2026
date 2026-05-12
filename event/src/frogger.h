#ifndef FROGGER_H
#define FROGGER_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

#define FROG_LANES     6
#define FROG_START_ROW 7   /* center of the 16-wide display */
#define FROG_START_COL 7   /* bottom row (col 7 = visual bottom in horizontal mode) */
#define FROG_GOAL_COL  0   /* top row = goal */

/*
 * Horizontal orientation: row=X (0-15 left→right), col=Y (0=top, 7=bottom).
 *
 * Layout:
 *   col 0       — goal (reach here to score)
 *   cols 1-6    — traffic lanes (cars move left or right)
 *   col 7       — safe start zone
 *
 * Controls:
 *   A = hop up   (col--)     D = hop down  (col++)
 *   B = hop left (row--)     C = hop right (row++)
 *   MENU (long)              = exit to menu
 *
 * Cars are bitmasks (bit N = row N occupied). Lanes rotate the mask
 * each step to simulate continuous traffic.
 */

typedef struct {
    uint16_t mask;       /* bit N = car at display row N */
    uint32_t last_tick;
    uint32_t ms;         /* ms per step */
    int8_t   dir;        /* +1=right (mask<<), -1=left (mask>>) */
} FrogLane;

typedef struct {
    FrogLane lane[FROG_LANES];
    int8_t   frog_row;
    int8_t   frog_col;
    uint8_t  lives;
    uint16_t score;
    bool     game_over;
} Frogger;

void frogger_init(Frogger *f);
/* Returns true when the player exits to the menu. */
bool frogger_update(Frogger *f, CBTS_MATRIX *display);

#endif /* FROGGER_H */
