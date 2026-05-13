#include "life.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include <string.h>

#define LIFE_W   16
#define LIFE_H   8
#define LIFE_MS  150

static bool ev_exit;
static bool ev_any;
static bool life_ready;   /* false until callback is registered */

static void life_key_cb(uint16_t key, uint8_t state) {
    if (key == HAL_KEY_SW_MENU) {
        if (state == HAL_KEY_EVENT_LONG) ev_exit = true;
        return;
    }
    if (state == HAL_KEY_EVENT_DOWN) ev_any = true;
}

/* ---- RNG ---- */

static uint16_t life_rng;

static uint16_t life_rand(void) {
    life_rng ^= life_rng << 7;
    life_rng ^= life_rng >> 9;
    life_rng ^= life_rng << 8;
    return life_rng;
}

/* ---- Life logic ---- */

static uint8_t count_neighbors(const uint16_t *grid, int x, int y) {
    int xl = (x + LIFE_W - 1) % LIFE_W;
    int xr = (x + 1) % LIFE_W;
    int yu = (y + LIFE_H - 1) % LIFE_H;
    int yd = (y + 1) % LIFE_H;
    return ((grid[yu] >> xl) & 1) + ((grid[yu] >> x) & 1) + ((grid[yu] >> xr) & 1) +
           ((grid[y]  >> xl) & 1) +                          ((grid[y]  >> xr) & 1) +
           ((grid[yd] >> xl) & 1) + ((grid[yd] >> x) & 1) + ((grid[yd] >> xr) & 1);
}

static void step(Life *l) {
    for (int y = 0; y < LIFE_H; y++) {
        l->next[y] = 0;
        for (int x = 0; x < LIFE_W; x++) {
            uint8_t n     = count_neighbors(l->grid, x, y);
            uint8_t alive = (l->grid[y] >> x) & 1;
            if ((alive && (n == 2 || n == 3)) || (!alive && n == 3))
                l->next[y] |= (uint16_t)(1u << x);
        }
    }
    memcpy(l->grid, l->next, sizeof(l->grid));
    l->generation++;
}

static bool all_dead(const Life *l) {
    for (int y = 0; y < LIFE_H; y++)
        if (l->grid[y]) return false;
    return true;
}

static void render(const Life *l, CBTS_MATRIX *display) {
    static const enum LedColor COLORS[] = {
        GREEN, CYAN, YELLOW, RED, MAGENTA, BLUE, WHITE
    };
    enum LedColor color = COLORS[(l->generation / 8) % 7];

    CBTS_MATRIX_clear(display);
    for (int y = 0; y < LIFE_H; y++) {
        for (int x = 0; x < LIFE_W; x++) {
            if ((l->grid[y] >> x) & 1)
                CBTS_MATRIX_setLedWithColor(display, x, y, color, true);
        }
    }
    CBTS_MATRIX_show(display);
}

static void randomize(Life *l) {
    life_rng = (uint16_t)(HAL_get_tick() | 1);
    for (int y = 0; y < LIFE_H; y++)
        l->grid[y] = life_rand();
    l->generation = 0;
    l->last_tick   = HAL_get_tick();
}

/* ---- Public API ---- */

void life_init(Life *l) {
    randomize(l);
    ev_exit    = false;
    ev_any     = false;
    life_ready = true;
    HalKeyConfig(life_key_cb);
}

bool life_update(Life *l, CBTS_MATRIX *display) {
    /* Glitch entry path: arrived here without life_init, set up input only.
     * The grid is intentionally left as-is (contains the previous game's data). */
    if (!life_ready) {
        ev_exit      = false;
        ev_any       = false;
        l->last_tick = HAL_get_tick();
        life_ready   = true;
        HalKeyConfig(life_key_cb);
    }

    if (ev_exit) {
        life_ready = false;
        return true;
    }

    if (ev_any) {
        ev_any = false;
        randomize(l);
    }

    uint32_t now = HAL_get_tick();
    if (now - l->last_tick >= LIFE_MS) {
        l->last_tick = now;
        step(l);
        if (all_dead(l)) randomize(l);
    }

    render(l, display);
    return false;
}
