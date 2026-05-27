#ifndef TICTAC_H
#define TICTAC_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

typedef struct {
    uint8_t board[9];
    uint8_t cursor_row;
    uint8_t cursor_col;
    uint8_t current_player;  /* 1 = X (RED), 2 = O (BLUE) */
    bool    game_over;
} TicTac;

void tictac_init(TicTac *g);
bool tictac_update(TicTac *g, CBTS_MATRIX *display);

#endif /* TICTAC_H */
