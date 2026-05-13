#include "frogger.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include "stats.h"

/* ---- Lane configuration ---- */

static const int8_t   LANE_DIR [FROG_LANES] = {  1, -1,  1, -1,  1, -1 };
static const uint32_t LANE_MS_0[FROG_LANES] = { 1200, 1000, 900, 1100, 800, 1300 };
static const uint16_t LANE_INIT[FROG_LANES] = {
    0xC30C,  /* rows 2-3, 8-9, 14-15  — three 2-px cars */
    0x30C3,  /* rows 0-1, 6-7, 12-13  — three 2-px cars */
    0xC0C0,  /* rows 6-7, 14-15       — two  2-px cars  */
    0x0C0C,  /* rows 2-3, 10-11       — two  2-px cars  */
    0xCC00,  /* rows 10-11, 14-15     — two  2-px cars  */
    0x0C30,  /* rows 4-5, 10-11       — two  2-px cars  */
};
static const enum LedColor LANE_COLOR[FROG_LANES] = {
    RED, YELLOW, RED, YELLOW, RED, YELLOW
};

/* ---- Input ---- */
static uint8_t ev_dir;
static bool    ev_exit, ev_any;

#define DIR_UP    0x01
#define DIR_DOWN  0x02
#define DIR_LEFT  0x04
#define DIR_RIGHT 0x08

static void frog_key_cb(uint16_t key, uint8_t state) {
    if (key == HAL_KEY_SW_MENU) {
        if (state == HAL_KEY_EVENT_LONG) ev_exit = true;
        return;
    }
    if (state != HAL_KEY_EVENT_DOWN) return;
    ev_any = true;
    if (key == HAL_KEY_SW_A) ev_dir |= DIR_UP;
    if (key == HAL_KEY_SW_D) ev_dir |= DIR_DOWN;
    if (key == HAL_KEY_SW_B) ev_dir |= DIR_LEFT;
    if (key == HAL_KEY_SW_C) ev_dir |= DIR_RIGHT;
}

/* ---- Helpers ---- */

static void respawn(Frogger *f) {
    f->frog_row = FROG_START_ROW;
    f->frog_col = FROG_START_COL;
}

static void step_lane(FrogLane *lane) {
    if (lane->dir > 0)
        lane->mask = (uint16_t)((lane->mask << 1) | (lane->mask >> 15));
    else
        lane->mask = (uint16_t)((lane->mask >> 1) | (lane->mask << 15));
}

static bool frog_in_danger(const Frogger *f) {
    int col = f->frog_col;
    if (col < 1 || col > FROG_LANES) return false;
    return (f->lane[col - 1].mask >> f->frog_row) & 1;
}

static void render(const Frogger *f, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);

    /* Goal markers at col 0 (three "lily pads") */
    CBTS_MATRIX_setLedWithColor(display, 3,  FROG_GOAL_COL, CYAN, true);
    CBTS_MATRIX_setLedWithColor(display, 7,  FROG_GOAL_COL, CYAN, true);
    CBTS_MATRIX_setLedWithColor(display, 11, FROG_GOAL_COL, CYAN, true);

    /* Traffic lanes */
    for (int l = 0; l < FROG_LANES; l++) {
        for (int r = 0; r < 16; r++) {
            if ((f->lane[l].mask >> r) & 1)
                CBTS_MATRIX_setLedWithColor(display, r, l + 1, LANE_COLOR[l], true);
        }
    }

    /* Lives (CYAN dots in start zone, upper-left corner) */
    for (uint8_t i = 0; i < f->lives; i++)
        CBTS_MATRIX_setLedWithColor(display, i, FROG_START_COL, CYAN, true);

    /* Frog — drawn last so it appears on top */
    CBTS_MATRIX_setLedWithColor(display, f->frog_row, f->frog_col, WHITE, true);

    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void frogger_init(Frogger *f) {
    uint32_t t = HAL_get_tick();
    for (int l = 0; l < FROG_LANES; l++) {
        f->lane[l].mask      = LANE_INIT[l];
        f->lane[l].ms        = LANE_MS_0[l];
        f->lane[l].dir       = LANE_DIR[l];
        f->lane[l].last_tick = t;
    }
    f->lives     = 3;
    f->score     = 0;
    f->game_over = false;
    respawn(f);

    ev_dir = 0; ev_exit = false; ev_any = false;
    HalKeyConfig(frog_key_cb);
}

bool frogger_update(Frogger *f, CBTS_MATRIX *display) {
    if (ev_exit) return true;

    uint32_t now = HAL_get_tick();

    /* Game over: frog blinks red, any key restarts */
    if (f->game_over) {
        CBTS_MATRIX_clear(display);
        if ((now / 300) & 1)
            CBTS_MATRIX_setLedWithColor(display, f->frog_row, f->frog_col, RED, true);
        CBTS_MATRIX_show(display);
        if (ev_any) { ev_any = false; frogger_init(f); }
        return false;
    }

    /* Advance lanes */
    for (int l = 0; l < FROG_LANES; l++) {
        if (now - f->lane[l].last_tick >= f->lane[l].ms) {
            f->lane[l].last_tick = now;
            step_lane(&f->lane[l]);
        }
    }

    /* Process hop (one cell per button press) */
    uint8_t dir = ev_dir; ev_dir = 0;
    if ((dir & DIR_UP)    && f->frog_col > FROG_GOAL_COL)  f->frog_col--;
    if ((dir & DIR_DOWN)  && f->frog_col < FROG_START_COL) f->frog_col++;
    if ((dir & DIR_LEFT)  && f->frog_row > 0)              f->frog_row--;
    if ((dir & DIR_RIGHT) && f->frog_row < 15)             f->frog_row++;

    /* Goal reached */
    if (f->frog_col == FROG_GOAL_COL) {
        f->score++;
        stats_record(f->score);
        if (f->score % 5 == 0) {
            /* Speed up all lanes by 15% every 5 crossings (min 80 ms) */
            for (int l = 0; l < FROG_LANES; l++) {
                uint32_t ms = f->lane[l].ms * 85 / 100;
                f->lane[l].ms = ms < 600 ? 600 : ms;
            }
        }
        ev_any = false;   /* discard key that triggered crossing */
        respawn(f);
    }

    /* Collision (car moved into frog or frog hopped onto car) */
    if (frog_in_danger(f)) {
        f->lives--;
        if (f->lives == 0) {
            f->game_over = true;
            ev_any = false;  /* prevent immediate restart */
        } else {
            respawn(f);
        }
    }

    render(f, display);
    return false;
}
