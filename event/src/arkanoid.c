#include "arkanoid.h"
#include "gpio_hal.h"
#include "systick_hal.h"

#define PADDLE_ROW      14
#define ARK_PADDLE_MS   60
#define INITIAL_BALL_MS 90
#define MIN_BALL_MS     40

static const enum LedColor BRICK_COLORS[ARK_BRICK_ROWS] = {
    RED, MAGENTA, YELLOW, GREEN, CYAN, BLUE
};

/* ---- Input ---- */
static bool   held_left, held_right, ev_launch, ev_exit, ev_any;
static int8_t serve_dc;

static void ark_key_cb(uint16_t key, uint8_t state) {
    if (key == HAL_KEY_SW_MENU) {
        if (state == HAL_KEY_EVENT_LONG) ev_exit = true;
        return;
    }
    if (state == HAL_KEY_EVENT_DOWN) {
        ev_any = true;
        if      (key == HAL_KEY_SW_C) held_left  = true;
        else if (key == HAL_KEY_SW_D) held_right = true;
        else                          ev_launch  = true;
    } else if (state == HAL_KEY_EVENT_UP) {
        if (key == HAL_KEY_SW_C) held_left  = false;
        if (key == HAL_KEY_SW_D) held_right = false;
    }
}

/* ---- Helpers ---- */

static void reset_bricks(Arkanoid *a) {
    for (int r = 0; r < ARK_BRICK_ROWS; r++)
        a->bricks[r] = 0xFF;
}

static bool bricks_clear(const Arkanoid *a) {
    for (int r = 0; r < ARK_BRICK_ROWS; r++)
        if (a->bricks[r]) return false;
    return true;
}

static bool brick_at(const Arkanoid *a, int r, int c) {
    if ((unsigned)r >= ARK_BRICK_ROWS || (unsigned)c >= ARK_W) return false;
    return (a->bricks[r] >> c) & 1;
}

static void break_brick(Arkanoid *a, int r, int c) {
    if ((unsigned)r >= ARK_BRICK_ROWS || (unsigned)c >= ARK_W) return;
    if (!((a->bricks[r] >> c) & 1)) return;
    a->bricks[r] &= ~(uint8_t)(1u << c);
    a->score++;
}

static void start_serving(Arkanoid *a) {
    a->serving = true;
    a->ball_r  = PADDLE_ROW - 2;
    a->ball_c  = a->paddle_x + ARK_PADDLE_W / 2;
    a->bdr     = -1;
    a->bdc     = serve_dc;
    serve_dc   = (int8_t)-serve_dc;
}

static void render(const Arkanoid *a, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);

    for (int r = 0; r < ARK_BRICK_ROWS; r++)
        for (int c = 0; c < ARK_W; c++)
            if ((a->bricks[r] >> c) & 1)
                CBTS_MATRIX_setLedWithColor(display, r, c, BRICK_COLORS[r], true);

    for (int i = 0; i < ARK_PADDLE_W; i++)
        CBTS_MATRIX_setLedWithColor(display, PADDLE_ROW, a->paddle_x + i, WHITE, true);

    if (!a->serving || (HAL_get_tick() / 200) & 1)
        CBTS_MATRIX_setLedWithColor(display, a->ball_r, a->ball_c, WHITE, true);

    for (uint8_t i = 0; i < a->lives; i++)
        CBTS_MATRIX_setLedWithColor(display, 15, i * 2, YELLOW, true);

    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void arkanoid_init(Arkanoid *a) {
    a->lives     = 3;
    a->score     = 0;
    a->level     = 0;
    a->ball_ms   = INITIAL_BALL_MS;
    a->paddle_x  = (ARK_W - ARK_PADDLE_W) / 2;
    a->game_over = false;
    reset_bricks(a);

    held_left = held_right = ev_launch = ev_exit = ev_any = false;
    serve_dc = 1;

    uint32_t t = HAL_get_tick();
    a->ball_last_tick   = t;
    a->paddle_last_tick = t;

    start_serving(a);
    HalKeyConfig(ark_key_cb);
}

bool arkanoid_update(Arkanoid *a, CBTS_MATRIX *display) {
    if (ev_exit) return true;

    uint32_t now = HAL_get_tick();

    if (a->game_over) {
        CBTS_MATRIX_clear(display);
        if ((now / 300) & 1)
            for (int r = 0; r < ARK_BRICK_ROWS; r++)
                for (int c = 0; c < ARK_W; c++)
                    CBTS_MATRIX_setLedWithColor(display, r, c, RED, true);
        CBTS_MATRIX_show(display);
        if (ev_any) { ev_any = false; arkanoid_init(a); }
        return false;
    }

    /* Paddle movement (held state, rate-limited) */
    if (now - a->paddle_last_tick >= ARK_PADDLE_MS) {
        a->paddle_last_tick = now;
        if (held_left  && a->paddle_x < ARK_W - ARK_PADDLE_W) a->paddle_x++;
        if (held_right && a->paddle_x > 0)                     a->paddle_x--;
    }

    /* Serving: ball tracks paddle center, any non-movement key launches */
    if (a->serving) {
        a->ball_c = a->paddle_x + ARK_PADDLE_W / 2;
        if (ev_launch) { ev_launch = false; a->serving = false; a->ball_last_tick = now; }
        render(a, display);
        return false;
    }
    ev_launch = false;

    /* Ball step (time-gated) */
    if (now - a->ball_last_tick < a->ball_ms) {
        render(a, display);
        return false;
    }
    a->ball_last_tick = now;

    int8_t nr = (int8_t)(a->ball_r + a->bdr);
    int8_t nc = (int8_t)(a->ball_c + a->bdc);

    /* Brick collision (checked before wall clamping to avoid false hits at boundary) */
    bool hit_r = brick_at(a, nr, a->ball_c);   /* crossed a row boundary */
    bool hit_c = brick_at(a, a->ball_r, nc);   /* crossed a col boundary */
    bool hit_d = brick_at(a, nr, nc);          /* diagonal / corner */

    if (hit_r) { break_brick(a, nr, a->ball_c); a->bdr = -a->bdr; nr = a->ball_r; }
    if (hit_c) { break_brick(a, a->ball_r, nc); a->bdc = -a->bdc; nc = a->ball_c; }
    if (!hit_r && !hit_c && hit_d) {
        break_brick(a, nr, nc);
        a->bdr = -a->bdr;
        a->bdc = -a->bdc;
        nr = a->ball_r;
        nc = a->ball_c;
    }

    /* Wall bounces */
    if (nc < 0)      { nc = 0;         a->bdc =  1; }
    if (nc >= ARK_W) { nc = ARK_W - 1; a->bdc = -1; }
    if (nr < 0)      { nr = 0;         a->bdr =  1; }

    /* Paddle collision / miss */
    if (nr >= PADDLE_ROW) {
        if (nr == PADDLE_ROW && nc >= a->paddle_x && nc < a->paddle_x + ARK_PADDLE_W) {
            int8_t hit_pos = (int8_t)(nc - a->paddle_x);
            a->bdr = -1;
            if (hit_pos == 0)                     a->bdc = -1;
            else if (hit_pos == ARK_PADDLE_W - 1) a->bdc =  1;
            nr = PADDLE_ROW - 1;
        } else {
            a->lives--;
            if (a->lives == 0) a->game_over = true;
            else               start_serving(a);
            render(a, display);
            return false;
        }
    }

    a->ball_r = nr;
    a->ball_c = nc;

    /* Level complete */
    if (bricks_clear(a)) {
        a->level++;
        if (a->ball_ms > MIN_BALL_MS + 10) a->ball_ms -= 10;
        reset_bricks(a);
        start_serving(a);
    }

    render(a, display);
    return false;
}
