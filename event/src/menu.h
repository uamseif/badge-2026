#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include "cbts_matrix.h"

typedef enum {
    APP_MARQUEE = 0,
    APP_MENU,
    APP_TETRIS,
    APP_PONG,
    APP_SNAKE,
    APP_FLAPPY,
    APP_SINVADERS,
    APP_FROGGER,
    APP_LIFE        = '8',  /* ASCII '8' = 56: magic char that triggers Life */
    APP_NAME_EDITOR = 9,

} AppState;

void marquee_init(void);
bool marquee_update(CBTS_MATRIX *display);

void menu_init(void);
AppState menu_update(CBTS_MATRIX *display);

#endif /* MENU_H */
