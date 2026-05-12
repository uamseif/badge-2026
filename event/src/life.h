#ifndef LIFE_H
#define LIFE_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

/*
 * Conway's Game of Life on a 16×8 toroidal grid.
 * grid[y] bit x = cell alive at column x, row y.
 */
typedef struct {
    uint16_t grid[8];
    uint16_t next[8];
    uint8_t  generation;
    uint32_t last_tick;
} Life;

void life_init(Life *l);
bool life_update(Life *l, CBTS_MATRIX *display);

#endif /* LIFE_H */
