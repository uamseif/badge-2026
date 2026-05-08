#include "snake.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include <string.h>

/* Row/col deltas for each direction: RIGHT, DOWN, LEFT, UP */
static const int8_t DR[4] = { 1,  0, -1,  0 };
static const int8_t DC[4] = { 0,  1,  0, -1 };

/* ---- RNG ---- */
static uint8_t rng_state = 1;

static uint8_t rng_next(void) {
    rng_state ^= (uint8_t)(rng_state << 7);
    rng_state ^= (uint8_t)(rng_state >> 5);
    rng_state ^= (uint8_t)(rng_state << 3);
    return rng_state;
}

/* ---- Input (set by callback) ---- */
static int8_t ev_turn;  /* -1=turn left, +1=turn right, 0=none */
static bool   ev_exit;
static bool   ev_any;

static void snake_key_cb(uint16_t key, uint8_t state) {
    if (state != HAL_KEY_EVENT_DOWN) return;
    if (key == HAL_KEY_SW_B)    ev_turn = -1;
    if (key == HAL_KEY_SW_C)    ev_turn =  1;
    if (key == HAL_KEY_SW_MENU) ev_exit = true;
    else                        ev_any  = true;
}

/* ---- Helpers ---- */

static uint32_t calc_step_ms(uint16_t score) {
    uint32_t ms = 200 - (uint32_t)(score > 30 ? 30 : score) * 4;
    return (ms < 80) ? 80 : ms;
}

static void place_food(Snake *s) {
    int8_t r, c;
    bool ok;
    do {
        r  = (int8_t)(rng_next() % SNAKE_W);
        c  = (int8_t)(rng_next() % SNAKE_H);
        ok = true;
        for (uint8_t i = 0; i < s->len; i++) {
            if (s->body_r[i] == r && s->body_c[i] == c) { ok = false; break; }
        }
    } while (!ok);
    s->food_r = r;
    s->food_c = c;
}

static void render(const Snake *s, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);
    /* Food */
    CBTS_MATRIX_setLedWithColor(display, s->food_r, s->food_c, RED, true);
    /* Body (tail to neck, drawn first so head is on top) */
    for (uint8_t i = s->len - 1; i > 0; i--)
        CBTS_MATRIX_setLedWithColor(display, s->body_r[i], s->body_c[i], GREEN, true);
    /* Head */
    CBTS_MATRIX_setLedWithColor(display, s->body_r[0], s->body_c[0], WHITE, true);
    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void snake_init(Snake *s) {
    memset(s, 0, sizeof(Snake));
    rng_state = (uint8_t)(HAL_get_tick() | 1);

    /* Start length 3, facing RIGHT, centered on the board */
    s->len       = 3;
    s->dir       = 0;                /* RIGHT */
    s->body_r[0] = 8; s->body_c[0] = 4;   /* head   */
    s->body_r[1] = 7; s->body_c[1] = 4;
    s->body_r[2] = 6; s->body_c[2] = 4;   /* tail   */
    s->step_ms   = calc_step_ms(0);
    s->last_tick = HAL_get_tick();

    ev_turn = 0;
    ev_exit = false;
    ev_any  = false;

    place_food(s);
    HalKeyConfig(snake_key_cb);
}

bool snake_update(Snake *s, CBTS_MATRIX *display) {
    if (ev_exit) return true;

    uint32_t now = HAL_get_tick();

    /* Game over: flash snake RED, any key restarts */
    if (s->game_over) {
        CBTS_MATRIX_clear(display);
        if ((now / 300) & 1) {
            for (uint8_t i = 0; i < s->len; i++)
                CBTS_MATRIX_setLedWithColor(display, s->body_r[i], s->body_c[i], RED, true);
        }
        CBTS_MATRIX_show(display);
        if (ev_any) { ev_any = false; snake_init(s); }
        return false;
    }

    /* Apply pending turn (only one per frame, last press wins) */
    if (ev_turn != 0) {
        if (ev_turn < 0) s->dir = (s->dir + 3) & 3;  /* turn left  */
        else             s->dir = (s->dir + 1) & 3;  /* turn right */
        ev_turn = 0;
    }

    /* Step at current speed */
    if (now - s->last_tick < s->step_ms) {
        render(s, display);
        return false;
    }
    s->last_tick = now;

    int8_t nr = s->body_r[0] + DR[s->dir];
    int8_t nc = s->body_c[0] + DC[s->dir];

    /* Wall collision */
    if (nr < 0 || nr >= SNAKE_W || nc < 0 || nc >= SNAKE_H) {
        s->game_over = true;
        render(s, display);
        return false;
    }

    /* Self collision (ignore tail — it will move this step) */
    for (uint8_t i = 0; i < s->len - 1; i++) {
        if (s->body_r[i] == nr && s->body_c[i] == nc) {
            s->game_over = true;
            render(s, display);
            return false;
        }
    }

    bool eating = (nr == s->food_r && nc == s->food_c);

    if (eating && s->len < SNAKE_MAXLEN) {
        /* Grow: insert head, keep tail */
        memmove(&s->body_r[1], &s->body_r[0], s->len);
        memmove(&s->body_c[1], &s->body_c[0], s->len);
        s->len++;
        s->score++;
        s->step_ms = calc_step_ms(s->score);
        place_food(s);
    } else {
        /* Move: insert head, drop tail */
        memmove(&s->body_r[1], &s->body_r[0], s->len - 1);
        memmove(&s->body_c[1], &s->body_c[0], s->len - 1);
    }
    s->body_r[0] = nr;
    s->body_c[0] = nc;

    render(s, display);
    return false;
}
