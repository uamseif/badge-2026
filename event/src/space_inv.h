#ifndef SPACE_INV_H
#define SPACE_INV_H

#include <stdint.h>
#include <stdbool.h>
#include "cbts_matrix.h"

#define SI_ROWS   3
#define SI_COLS   5
#define SI_TOTAL (SI_ROWS * SI_COLS)

/*
 * Vertical orientation: row=Y (0=top,15=bottom), col=X (0=visual-right, 7=visual-left).
 * C = move left (col++), D = move right (col--).
 * A or B = shoot. MENU (long press) = exit.
 *
 * Alien grid occupies cols alien_x to alien_x+SI_COLS-1.
 * Bounces off col walls and drops one row on each bounce.
 */

typedef struct {
    uint8_t  aliens[SI_ROWS];   /* bitmask: bit c = alien at col c alive */
    int8_t   alien_x;           /* leftmost display col of alien grid     */
    int8_t   alien_y;           /* topmost display row of alien grid      */
    int8_t   alien_dx;          /* lateral direction: +1 or -1            */
    int8_t   player_x;          /* player col (0-7)                       */
    int8_t   bullet_r;          /* player bullet row, -1 = inactive       */
    int8_t   bullet_c;
    int8_t   abul_r;            /* alien bullet row, -1 = inactive        */
    int8_t   abul_c;
    uint8_t  lives;
    uint8_t  level;
    uint16_t score;
    uint32_t alien_ms;          /* current ms per alien step              */
    uint8_t  rng;
    bool     game_over;
    bool     victory;
    uint32_t alien_last_tick;
    uint32_t bullet_last_tick;
    uint32_t abul_last_tick;
    uint32_t shoot_last_tick;
    uint32_t player_last_tick;
    uint32_t victory_tick;
} SpaceInvaders;

void si_init(SpaceInvaders *si);
/* Returns true when the player exits to the menu. */
bool si_update(SpaceInvaders *si, CBTS_MATRIX *display);

#endif /* SPACE_INV_H */
