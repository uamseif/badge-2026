#ifndef PONG_H
#define PONG_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

#define PONG_W          8
#define PONG_H          16
#define PONG_PADDLE_W   3
#define PONG_PADDLE_Y1  1    /* P1 paddle row (top)    */
#define PONG_PADDLE_Y2  14   /* P2 paddle row (bottom) */
#define PONG_MAX_SCORE  5

/*
 * Button layout — vertical orientation:
 *
 *  top row:    [ B (P1 left) ]  [ A (P1 right) ]
 *  bottom row: [ C (P2 left) ]  [ D (P2 right) ]
 *  side:         Menu = exit to menu / serve
 */

typedef struct {
    int8_t  p1x, p2x;          /* paddle left-edge column (0..PONG_W-PADDLE_W) */
    int8_t  bx, by;            /* ball position                                 */
    int8_t  bdx, bdy;          /* ball direction (-1/0/+1, -1/+1)               */
    uint8_t p1_score;
    uint8_t p2_score;
    bool    serving;            /* true = waiting for button to launch ball      */
    uint8_t server;             /* 0 = P1 serves, 1 = P2 serves                 */
    bool    game_over;
    uint8_t winner;             /* 0 = P1, 1 = P2                               */
    uint32_t last_ball_tick;
    uint32_t last_paddle_tick;
} Pong;

void pong_init(Pong *p);
/* Returns true when the player exits to the menu. */
bool pong_update(Pong *p, CBTS_MATRIX *display);

#endif /* PONG_H */
