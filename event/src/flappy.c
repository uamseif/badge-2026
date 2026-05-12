#include "flappy.h"
#include "gpio_hal.h"
#include "systick_hal.h"

#define TICK_MS        150   /* ms per game step */
#define PIPE_INTERVAL  8     /* ticks between pipe spawns */

/* ---- Input ---- */
static bool ev_flap, ev_exit, ev_any;

static void flappy_key_cb(uint16_t key, uint8_t state) {
    if (key == HAL_KEY_SW_MENU) {
        if (state == HAL_KEY_EVENT_LONG) ev_exit = true;
        return;
    }
    if (state != HAL_KEY_EVENT_DOWN) return;
    ev_any = true;
    ev_flap = true;
}

/* ---- RNG (LFSR, state stored in struct) ---- */
static uint8_t rng_next(uint8_t *s) {
    *s ^= (uint8_t)(*s << 7);
    *s ^= (uint8_t)(*s >> 5);
    *s ^= (uint8_t)(*s << 3);
    return *s;
}

/* ---- Helpers ---- */

static void spawn_pipe(Flappy *f) {
    for (int i = 0; i < FLAPPY_PIPES; i++) {
        if (f->pipe_x[i] < 0) {
            f->pipe_x[i]   = 15;
            /* gap top: 1-4 so both top and bottom pipe have >= 1 pixel */
            f->pipe_gap[i] = (int8_t)(1 + rng_next(&f->rng) % (8 - FLAPPY_GAP - 1));
            return;
        }
    }
}

static void render(const Flappy *f, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);

    for (int i = 0; i < FLAPPY_PIPES; i++) {
        if (f->pipe_x[i] < 0) continue;
        int x = f->pipe_x[i];
        for (int c = 0; c < 8; c++) {
            if (c < f->pipe_gap[i] || c >= f->pipe_gap[i] + FLAPPY_GAP)
                CBTS_MATRIX_setLedWithColor(display, x, c, GREEN, true);
        }
    }

    /* Bird blinks red on game over */
    if (!f->game_over || (HAL_get_tick() / 200) & 1) {
        enum LedColor bird_col = f->game_over ? RED : YELLOW;
        CBTS_MATRIX_setLedWithColor(display, BIRD_X, f->bird_y, bird_col, true);
    }

    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void flappy_init(Flappy *f) {
    f->bird_y    = 3;
    f->bird_vy   = 0;
    f->score     = 0;
    f->tick_count = 0;
    f->game_over = false;
    f->rng       = (uint8_t)(HAL_get_tick() | 1);
    f->last_tick = HAL_get_tick();

    for (int i = 0; i < FLAPPY_PIPES; i++) f->pipe_x[i] = -1;

    /* Pre-spawn first pipe so it's visible immediately */
    spawn_pipe(f);

    ev_flap = ev_exit = ev_any = false;
    HalKeyConfig(flappy_key_cb);
}

bool flappy_update(Flappy *f, CBTS_MATRIX *display) {
    if (ev_exit) return true;

    if (f->game_over) {
        render(f, display);
        if (ev_any) { ev_any = false; flappy_init(f); }
        return false;
    }

    /* Apply flap velocity immediately (visual update on next tick) */
    if (ev_flap) { ev_flap = false; f->bird_vy = -2; }

    uint32_t now = HAL_get_tick();
    if (now - f->last_tick < TICK_MS) {
        render(f, display);
        return false;
    }
    f->last_tick = now;
    f->tick_count++;

    /* Bird physics */
    f->bird_vy = (int8_t)(f->bird_vy < 2 ? f->bird_vy + 1 : 2);
    f->bird_y  = (int8_t)(f->bird_y + f->bird_vy);

    if (f->bird_y < 0 || f->bird_y > 7) {
        f->bird_y    = (int8_t)(f->bird_y < 0 ? 0 : 7);
        f->game_over = true;
        render(f, display);
        return false;
    }

    /* Move pipes, score, collision */
    for (int i = 0; i < FLAPPY_PIPES; i++) {
        if (f->pipe_x[i] < 0) continue;
        f->pipe_x[i]--;
        if (f->pipe_x[i] < 0) { f->pipe_x[i] = -1; continue; }

        /* Score: pipe just cleared the bird */
        if (f->pipe_x[i] == BIRD_X - 1) f->score++;

        /* Collision: pipe aligned with bird */
        if (f->pipe_x[i] == BIRD_X) {
            int g = f->pipe_gap[i];
            if (f->bird_y < g || f->bird_y >= g + FLAPPY_GAP) {
                f->game_over = true;
                render(f, display);
                return false;
            }
        }
    }

    /* Spawn new pipe on interval */
    if (f->tick_count % PIPE_INTERVAL == 0) spawn_pipe(f);

    render(f, display);
    return false;
}
