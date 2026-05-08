#ifndef CBTS_TETRIS_TETRIS_H
#define CBTS_TETRIS_TETRIS_H
#include "stdio.h"

#define NUM_PIECE_TYPES     7
#define PIECE_W             4
#define PIECE_H             4

#define GRID_W              8
#define GRID_H              16

void TETRIS_loop(uint8_t* framebuffer);
void TETRIS_setup();
void TETRIS_key_callback(uint16_t key, uint8_t state);
#endif //CBTS_TETRIS_TETRIS_H
