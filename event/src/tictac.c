#include "tictac.h"
#include "gpio_hal.h"
#include "systick_hal.h"

#define EMPTY    0
#define PLAYER_X 1
#define PLAYER_O 2

/* ---- Display layout ---- */

static const uint8_t WIN_LINES[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8},
    {0,3,6}, {1,4,7}, {2,5,8},
    {0,4,8}, {2,4,6},
};

static const uint8_t CELL_L[3] = { 0,  6, 11};
static const uint8_t CELL_R[3] = { 4,  9, 15};
static const uint8_t CELL_T[3] = { 0,  3,  6};
static const uint8_t CELL_B[3] = { 1,  4,  7};

/* ---- Input ---- */

static uint8_t ev_dir;
static bool    ev_confirm, ev_exit, ev_any;

#define DIR_UP    0x01
#define DIR_DOWN  0x02
#define DIR_LEFT  0x04
#define DIR_RIGHT 0x08

static void tictac_key_cb(uint16_t key, uint8_t state) {
    if (key == HAL_KEY_SW_MENU) {
        if (state == HAL_KEY_EVENT_DOWN) ev_confirm = true;
        if (state == HAL_KEY_EVENT_LONG) { ev_confirm = false; ev_exit = true; }
        return;
    }
    if (state != HAL_KEY_EVENT_DOWN) return;
    ev_any = true;
    if (key == HAL_KEY_SW_A) ev_dir |= DIR_UP;
    if (key == HAL_KEY_SW_D) ev_dir |= DIR_DOWN;
    if (key == HAL_KEY_SW_B) ev_dir |= DIR_LEFT;
    if (key == HAL_KEY_SW_C) ev_dir |= DIR_RIGHT;
}

/* ---- Game logic ---- */

static uint8_t cidx(uint8_t row, uint8_t col) { return (uint8_t)(row * 3u + col); }

static bool has_won(const uint8_t *b, uint8_t piece) {
    for (uint8_t i = 0; i < 8; i++)
        if (b[WIN_LINES[i][0]] == piece &&
            b[WIN_LINES[i][1]] == piece &&
            b[WIN_LINES[i][2]] == piece) return true;
    return false;
}

static bool board_full(const uint8_t *b) {
    for (uint8_t i = 0; i < 9; i++)
        if (b[i] == EMPTY) return false;
    return true;
}

/* ---- Cursor helpers ---- */

static void snap_to_empty(TicTac *g) {
    uint8_t start = cidx(g->cursor_row, g->cursor_col);
    if (g->board[start] == EMPTY) return;
    for (uint8_t off = 1; off <= 9; off++) {
        uint8_t idx = (uint8_t)((start + off) % 9u);
        if (g->board[idx] == EMPTY) {
            g->cursor_row = (uint8_t)(idx / 3u);
            g->cursor_col = (uint8_t)(idx % 3u);
            return;
        }
    }
}

static uint8_t wrap3(uint8_t v, int8_t d) {
    if (d < 0) return (v == 0) ? 2u : (uint8_t)(v - 1u);
    if (d > 0) return (v == 2) ? 0u : (uint8_t)(v + 1u);
    return v;
}

static void cursor_step(TicTac *g, int8_t dr, int8_t dc) {
    uint8_t r = g->cursor_row, c = g->cursor_col;
    /* 2 iterations: covers all cells in this axis except the starting one */
    for (uint8_t s = 0; s < 2; s++) {
        r = wrap3(r, dr);
        c = wrap3(c, dc);
        if (g->board[cidx(r, c)] == EMPTY) { g->cursor_row = r; g->cursor_col = c; return; }
    }
    /* Fallback: full scan — find any empty cell other than current */
    uint8_t start = cidx(g->cursor_row, g->cursor_col);
    for (uint8_t off = 1; off <= 9; off++) {
        uint8_t idx = (uint8_t)((start + off) % 9u);
        if (g->board[idx] == EMPTY) {
            g->cursor_row = (uint8_t)(idx / 3u);
            g->cursor_col = (uint8_t)(idx % 3u);
            return;
        }
    }
}

static void move_cursor(TicTac *g, uint8_t dir) {
    if (dir & DIR_UP)    cursor_step(g, -1,  0);
    if (dir & DIR_DOWN)  cursor_step(g,  1,  0);
    if (dir & DIR_LEFT)  cursor_step(g,  0, -1);
    if (dir & DIR_RIGHT) cursor_step(g,  0,  1);
}

/* ---- Render ---- */

static void fill_cell(CBTS_MATRIX *display, uint8_t row, uint8_t col, enum LedColor color) {
    for (uint8_t x = CELL_L[col]; x <= CELL_R[col]; x++)
        for (uint8_t y = CELL_T[row]; y <= CELL_B[row]; y++)
            CBTS_MATRIX_setLedWithColor(display, x, y, color, true);
}

static void render(const TicTac *g, CBTS_MATRIX *display, uint32_t now) {
    CBTS_MATRIX_clear(display);

    for (uint8_t y = 0; y < 8; y++) {
        CBTS_MATRIX_setLedWithColor(display,  5, y, WHITE, true);
        CBTS_MATRIX_setLedWithColor(display, 10, y, WHITE, true);
    }
    for (uint8_t x = 0; x < 16; x++) {
        CBTS_MATRIX_setLedWithColor(display, x, 2, WHITE, true);
        CBTS_MATRIX_setLedWithColor(display, x, 5, WHITE, true);
    }

    for (uint8_t r = 0; r < 3; r++) {
        for (uint8_t c = 0; c < 3; c++) {
            uint8_t piece = g->board[cidx(r, c)];
            if (piece == PLAYER_X) fill_cell(display, r, c, RED);
            if (piece == PLAYER_O) fill_cell(display, r, c, BLUE);
        }
    }

    /* Cursor blink: player's own color */
    if (!g->game_over && (now / 250) & 1) {
        if (g->board[cidx(g->cursor_row, g->cursor_col)] == EMPTY) {
            enum LedColor cur_color = (g->current_player == 1) ? RED : BLUE;
            fill_cell(display, g->cursor_row, g->cursor_col, cur_color);
        }
    }

    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void tictac_init(TicTac *g) {
    for (uint8_t i = 0; i < 9; i++) g->board[i] = EMPTY;
    g->cursor_row    = 1;
    g->cursor_col    = 1;
    g->current_player = 1;
    g->game_over     = false;
    ev_dir = 0; ev_confirm = false; ev_exit = false; ev_any = false;
    HalKeyConfig(tictac_key_cb);
}

bool tictac_update(TicTac *g, CBTS_MATRIX *display) {
    if (ev_exit) return true;

    uint32_t now = HAL_get_tick();

    /* Game over: blink board, any key = restart */
    if (g->game_over) {
        if ((now / 300) & 1) render(g, display, now);
        else { CBTS_MATRIX_clear(display); CBTS_MATRIX_show(display); }
        if (ev_any) { ev_any = false; tictac_init(g); }
        return false;
    }

    uint8_t dir     = ev_dir;     ev_dir     = 0;
    bool    confirm = ev_confirm; ev_confirm = false;
    ev_any = false;

    if (dir) move_cursor(g, dir);

    if (confirm) {
        uint8_t idx = cidx(g->cursor_row, g->cursor_col);
        if (g->board[idx] == EMPTY) {
            uint8_t piece = (g->current_player == 1) ? PLAYER_X : PLAYER_O;
            g->board[idx] = piece;
            if (has_won(g->board, piece) || board_full(g->board))
                g->game_over = true;
            else {
                g->current_player = (g->current_player == 1) ? 2 : 1;
                snap_to_empty(g);
            }
        }
    }

    render(g, display, now);
    return false;
}
