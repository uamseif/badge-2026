#ifndef MENU_H
#define MENU_H

/* Uncomment to build a speaker badge (blue-only marquee).
 * Leave commented for attendee badge (rainbow minus blue). */
//#define IS_SPEAKER

#include <stdbool.h>
#include "cbts_matrix.h"

typedef enum {
    APP_MARQUEE     = '0',
    APP_MENU        = '1',
    APP_TETRIS      = '2',
    APP_PONG        = '3',
    APP_SNAKE       = '4',
    APP_FLAPPY      = '5',
    APP_SINVADERS   = '6',
    APP_FROGGER     = '7',
    APP_LIFE        = '8',
    APP_NAME_EDITOR  = '9',
    APP_TICTAC       = 'A',
} AppState;

void marquee_init(void);
bool marquee_update(CBTS_MATRIX *display);

void menu_init(void);
AppState menu_update(CBTS_MATRIX *display);

#endif /* MENU_H */
