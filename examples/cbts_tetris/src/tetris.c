#include "tetris.h"
#include "gpio_hal.h"
#include "systick_hal.h"

uint8_t piece_x = 0;
uint8_t piece_y = 0;

uint8_t piece_rotation = 0;
uint8_t piece_id = 3;

uint32_t clock = 0;
uint32_t fall_clock;

const char piece_I[] = {
        0,0,0,0,
        1,1,1,1,
        0,0,0,0,
        0,0,0,0,

        0,0,1,0,
        0,0,1,0,
        0,0,1,0,
        0,0,1,0,

        0,0,0,0,
        0,0,0,0,
        1,1,1,1,
        0,0,0,0,

        0,1,0,0,
        0,1,0,0,
        0,1,0,0,
        0,1,0,0,
};

const char piece_L[] = {
        0,0,1,0,
        1,1,1,0,
        0,0,0,0,
        0,0,0,0,

        0,1,0,0,
        0,1,0,0,
        0,1,1,0,
        0,0,0,0,

        0,0,0,0,
        1,1,1,0,
        1,0,0,0,
        0,0,0,0,

        1,1,0,0,
        0,1,0,0,
        0,1,0,0,
        0,0,0,0,
};

const char piece_J[] = {
        1,0,0,0,
        1,1,1,0,
        0,0,0,0,
        0,0,0,0,

        0,1,1,0,
        0,1,0,0,
        0,1,0,0,
        0,0,0,0,

        0,0,0,0,
        1,1,1,0,
        0,0,1,0,
        0,0,0,0,

        0,1,0,0,
        0,1,0,0,
        1,1,0,0,
        0,0,0,0,

};

const char piece_T[] = {
        0,1,0,0,
        1,1,1,0,
        0,0,0,0,
        0,0,0,0,

        0,1,0,0,
        0,1,1,0,
        0,1,0,0,
        0,0,0,0,

        0,0,0,0,
        1,1,1,0,
        0,1,0,0,
        0,0,0,0,

        0,1,0,0,
        1,1,0,0,
        0,1,0,0,
        0,0,0,0,

};

const char piece_S[] = {
        0,1,1,0,
        1,1,0,0,
        0,0,0,0,
        0,0,0,0,

        0,1,0,0,
        0,1,1,0,
        0,0,1,0,
        0,0,0,0,

        0,0,0,0,
        0,1,1,0,
        1,1,0,0,
        0,0,0,0,

        1,0,0,0,
        1,1,0,0,
        0,1,0,0,
        0,0,0,0,
};

const char piece_Z[] = {
        1,1,0,0,
        0,1,1,0,
        0,0,0,0,
        0,0,0,0,

        0,0,1,0,
        0,1,1,0,
        0,1,0,0,
        0,0,0,0,

        0,0,0,0,
        1,1,0,0,
        0,1,1,0,
        0,0,0,0,

        0,1,0,0,
        1,1,0,0,
        1,0,0,0,
        0,0,0,0,
};

const char piece_O[] = {
        1,1,0,0,
        1,1,0,0,
        0,0,0,0,
        0,0,0,0,

        1,1,0,0,
        1,1,0,0,
        0,0,0,0,
        0,0,0,0,

        1,1,0,0,
        1,1,0,0,
        0,0,0,0,
        0,0,0,0,

        1,1,0,0,
        1,1,0,0,
        0,0,0,0,
        0,0,0,0,
};

const char *pieces[NUM_PIECE_TYPES] = {
        piece_S,
        piece_Z,
        piece_L,
        piece_J,
        piece_O,
        piece_T,
        piece_I,
};

void next_piece() {
    piece_id = 2 % NUM_PIECE_TYPES;
    piece_x = 3;
    piece_y = -4;
}



void TETRIS_key_callback(uint16_t key, uint8_t state) {
    if(state == HAL_KEY_EVENT_DOWN) {
        if(key == HAL_KEY_SW_5) {
            piece_rotation++;
        }
    }

    if (piece_rotation >= 4) {
        piece_rotation = 0;
    }
}

void fall_piece() {
    if(HAL_get_tick() - clock > 500) {
        clock = HAL_get_tick();

        piece_y++;

        if(piece_y == GRID_H - PIECE_H) {
            next_piece();
        }
    }
}

void draw_piece(uint8_t* framebuffer) {

    const char *piece = pieces[piece_id] + (piece_rotation * PIECE_H * PIECE_W);

    for (uint8_t y = 0; y < PIECE_H; y++) {
        uint8_t row = 0;
        for (uint8_t x = 0; x < PIECE_W; x++) {
            row |= piece[(y * PIECE_H) + x] << (x + piece_x);
        }
        framebuffer[y + piece_y] = row;
    }
}

void TETRIS_loop(uint8_t* framebuffer) {
    fall_piece();
    draw_piece(framebuffer);

}
void TETRIS_setup() {
    next_piece();
    fall_clock = clock = HAL_get_tick();
}