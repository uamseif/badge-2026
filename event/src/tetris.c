#include "tetris.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include <string.h>

/* ---- Input events (set by key callback, consumed by update) ---- */
static uint8_t ev_pending;   /* one-shot actions: move, rotate, hard drop, exit */
static bool    ev_soft_drop; /* held state: true while soft-drop button is held */

#define EV_LEFT      0x01
#define EV_RIGHT     0x02
#define EV_ROTATE    0x04
#define EV_HARD_DROP 0x08
#define EV_EXIT      0x10

static void tetris_key_cb(uint16_t key, uint8_t state) {
    if (state == HAL_KEY_EVENT_LONG) {
        if (key == HAL_KEY_SW_MENU) ev_pending |= EV_EXIT;
        return;
    }
    if (state == HAL_KEY_EVENT_DOWN) {
        if (key == BTN_ROTATE)    ev_pending  |= EV_ROTATE;
        if (key == BTN_HARD_DROP) ev_pending  |= EV_HARD_DROP;
        if (key == BTN_SOFT_DROP) ev_soft_drop = true;
    }
    /* DOWN + LONG both trigger movement (LONG = auto-repeat) */
    if (state == HAL_KEY_EVENT_DOWN || state == HAL_KEY_EVENT_LONG) {
        if (key == BTN_LEFT)  ev_pending |= EV_LEFT;
        if (key == BTN_RIGHT) ev_pending |= EV_RIGHT;
    }
    if (state == HAL_KEY_EVENT_UP) {
        if (key == BTN_SOFT_DROP) ev_soft_drop = false;
    }
}

/* ---- Piece cell offsets (dx, dy) from piece origin, stored in flash ---- */

typedef struct { int8_t dx, dy; } PCell;

static const PCell SHAPES[TETRIS_PIECES][4][4] = {
    /* 0: I - CYAN */
    {{{0,1},{1,1},{2,1},{3,1}}, {{2,0},{2,1},{2,2},{2,3}},
     {{0,2},{1,2},{2,2},{3,2}}, {{1,0},{1,1},{1,2},{1,3}}},
    /* 1: O - YELLOW */
    {{{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}},
     {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}},
    /* 2: T - MAGENTA */
    {{{1,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{2,1},{1,2}},
     {{0,1},{1,1},{2,1},{1,2}}, {{1,0},{0,1},{1,1},{1,2}}},
    /* 3: S - GREEN */
    {{{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}},
     {{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}}},
    /* 4: Z - RED */
    {{{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}},
     {{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}}},
    /* 5: J - BLUE */
    {{{0,0},{0,1},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{1,2}},
     {{0,1},{1,1},{2,1},{2,2}}, {{1,0},{1,1},{0,2},{1,2}}},
    /* 6: L - WHITE */
    {{{2,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{1,2},{2,2}},
     {{0,1},{1,1},{2,1},{0,2}}, {{0,0},{1,0},{1,1},{1,2}}},
};

/* Board value = PIECE_COLOR[piece_idx]; 0 = empty */
static const uint8_t PIECE_COLOR[TETRIS_PIECES] = {
    CYAN+1, YELLOW+1, MAGENTA+1, GREEN+1, RED+1, BLUE+1, WHITE+1
};

static const uint16_t SCORE_TABLE[5] = {0, 40, 100, 300, 1200};

/* ---- RNG (8-bit Galois LFSR) ---- */
static uint8_t rng_state = 1;

static uint8_t rand_piece(void) {
    rng_state ^= (uint8_t)(rng_state << 7);
    rng_state ^= (uint8_t)(rng_state >> 5);
    rng_state ^= (uint8_t)(rng_state << 3);
    return rng_state % TETRIS_PIECES;
}

/* ---- Helpers ---- */

static uint32_t level_fall_ms(uint8_t level) {
    if (level >= 7) return 100;
    return (uint32_t)(800 - (uint32_t)level * 100);
}

static bool fits(const Tetris *t, int8_t px, int8_t py, int8_t rot) {
    const PCell *cells = SHAPES[t->piece][rot];
    for (uint8_t i = 0; i < 4; i++) {
        int8_t x = px + cells[i].dx;
        int8_t y = py + cells[i].dy;
        if (x < 0 || x >= TETRIS_W || y >= TETRIS_H) return false;
        if (y >= 0 && t->board[y][x]) return false;
    }
    return true;
}

static void lock_piece(Tetris *t) {
    uint8_t color = PIECE_COLOR[t->piece];
    const PCell *cells = SHAPES[t->piece][t->rot];
    for (uint8_t i = 0; i < 4; i++) {
        int8_t x = t->px + cells[i].dx;
        int8_t y = t->py + cells[i].dy;
        if (y >= 0 && y < TETRIS_H && x >= 0 && x < TETRIS_W)
            t->board[y][x] = color;
    }
}

static uint8_t clear_lines(Tetris *t) {
    uint8_t cleared = 0;
    for (int8_t row = TETRIS_H - 1; row >= 0; row--) {
        bool full = true;
        for (uint8_t col = 0; col < TETRIS_W; col++) {
            if (!t->board[row][col]) { full = false; break; }
        }
        if (!full) continue;
        for (int8_t r = row; r > 0; r--)
            memcpy(t->board[r], t->board[r - 1], TETRIS_W);
        memset(t->board[0], 0, TETRIS_W);
        row++;   /* recheck same row index after shift */
        cleared++;
    }
    return cleared;
}

static void do_lock_spawn(Tetris *t) {
    lock_piece(t);
    uint8_t cleared = clear_lines(t);
    if (cleared) {
        t->lines += cleared;
        t->score += SCORE_TABLE[cleared] * (uint16_t)(t->level + 1);
        t->level = (uint8_t)(t->lines / 10);
        if (t->level > 15) t->level = 15;
        t->fall_ms = level_fall_ms(t->level);
    }
    /* Spawn next piece */
    t->piece = t->next;
    t->next  = rand_piece();
    t->rot   = 0;
    t->px    = 2;
    t->py    = -1;
    if (!fits(t, t->px, t->py, t->rot))
        t->game_over = true;
}

static void render(const Tetris *t, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);

    /* Locked board */
    for (uint8_t row = 0; row < TETRIS_H; row++) {
        for (uint8_t col = 0; col < TETRIS_W; col++) {
            if (t->board[row][col]) {
                enum LedColor c = (enum LedColor)(t->board[row][col] - 1);
                CBTS_MATRIX_setLedWithColor(display, row, col, c, true);
            }
        }
    }

    /* Current piece */
    if (!t->game_over) {
        enum LedColor c = (enum LedColor)(PIECE_COLOR[t->piece] - 1);
        const PCell *cells = SHAPES[t->piece][t->rot];
        for (uint8_t i = 0; i < 4; i++) {
            int8_t x = t->px + cells[i].dx;
            int8_t y = t->py + cells[i].dy;
            if (y >= 0 && y < TETRIS_H && x >= 0 && x < TETRIS_W)
                CBTS_MATRIX_setLedWithColor(display, y, x, c, true);
        }
    }

    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void tetris_init(Tetris *t) {
    memset(t, 0, sizeof(Tetris));
    rng_state    = (uint8_t)(HAL_get_tick() | 1);
    t->fall_ms   = level_fall_ms(0);
    t->last_fall = HAL_get_tick();
    t->piece     = rand_piece();
    t->next      = rand_piece();
    t->px        = 2;
    t->py        = -1;
    /* Register input callback and clear any pending events */
    ev_pending   = 0;
    ev_soft_drop = false;
    HalKeyConfig(tetris_key_cb);
}

bool tetris_update(Tetris *t, CBTS_MATRIX *display) {
    /* Consume pending events atomically */
    uint8_t ev  = ev_pending;
    ev_pending  = 0;

    if (ev & EV_EXIT) return true;

    if (t->game_over) {
        /* Flash top row red; EXIT goes to menu, any other action restarts */
        uint32_t now = HAL_get_tick();
        CBTS_MATRIX_clear(display);
        if ((now / 300) & 1) {
            for (uint8_t col = 0; col < TETRIS_W; col++)
                CBTS_MATRIX_setLedWithColor(display, 0, col, RED, true);
        }
        CBTS_MATRIX_show(display);
        if (ev & EV_EXIT) return true;
        if (ev) tetris_init(t);
        return false;
    }

    /* Hard drop */
    if (ev & EV_HARD_DROP) {
        while (fits(t, t->px, t->py + 1, t->rot))
            t->py++;
        do_lock_spawn(t);
        render(t, display);
        return false;
    }

    /* Move left / right (DOWN + auto-repeat LONG both set EV_LEFT/RIGHT) */
    if (ev & EV_LEFT) {
        if (fits(t, t->px + 1, t->py, t->rot)) t->px++;
    }
    if (ev & EV_RIGHT) {
        if (fits(t, t->px - 1, t->py, t->rot)) t->px--;
    }

    /* Rotate with simple wall kick */
    if (ev & EV_ROTATE) {
        int8_t nr = (t->rot + 1) % 4;
        if      (fits(t, t->px,     t->py, nr)) { t->rot = nr; }
        else if (fits(t, t->px + 1, t->py, nr)) { t->px++; t->rot = nr; }
        else if (fits(t, t->px - 1, t->py, nr)) { t->px--; t->rot = nr; }
    }

    /* Gravity (soft drop halves the interval while held) */
    uint32_t interval = ev_soft_drop ? 50 : t->fall_ms;
    uint32_t now = HAL_get_tick();
    if (now - t->last_fall >= interval) {
        t->last_fall = now;
        if (fits(t, t->px, t->py + 1, t->rot)) {
            t->py++;
        } else {
            do_lock_spawn(t);
        }
    }

    render(t, display);
    return false;
}
