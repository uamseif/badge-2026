#ifndef FLAPPY_H
#define FLAPPY_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

#define FLAPPY_PIPES  3    /* max simultaneous pipes */
#define FLAPPY_GAP    3    /* gap height in pixels   */
#define BIRD_X        2    /* fixed display row (horizontal mode) */

/*
 * Horizontal orientation: row=X (0-15), col=Y (0=top, 7=bottom).
 * Any button except MENU = flap up. MENU = exit.
 */

typedef struct {
    int8_t   bird_y;                  /* vertical pos (col 0=top, 7=bottom) */
    int8_t   bird_vy;                 /* vertical velocity (-2 to +2)        */
    int8_t   pipe_x[FLAPPY_PIPES];   /* horizontal pos, -1 = inactive       */
    int8_t   pipe_gap[FLAPPY_PIPES]; /* top col of the gap (1-4)            */
    uint16_t score;
    uint8_t  rng;
    uint16_t tick_count;
    bool     game_over;
    uint32_t last_tick;
} Flappy;

void flappy_init(Flappy *f);
/* Returns true when the player exits to the menu. */
bool flappy_update(Flappy *f, CBTS_MATRIX *display);

#endif /* FLAPPY_H */
