#include "pong.h"
#include "gpio_hal.h"
#include "systick_hal.h"

#define BALL_MS     90    /* ms between ball steps          */
#define PADDLE_MS   60    /* ms between paddle steps (held) */

#define P1_COLOR    BLUE
#define P2_COLOR    RED
#define BALL_COLOR  WHITE

/* ---- Input state (set by callback) ---- */
static bool p1_left, p1_right;
static bool p2_left, p2_right;
static bool ev_any;   /* any non-menu button pressed */
static bool ev_exit;  /* MENU button pressed         */

static void pong_key_cb(uint16_t key, uint8_t state) {
    if (key == HAL_KEY_SW_MENU) {
        if (state == HAL_KEY_EVENT_LONG) ev_exit = true;
        return;
    }
    if (state == HAL_KEY_EVENT_DOWN) {
        if (key == HAL_KEY_SW_B) p1_left  = true;
        if (key == HAL_KEY_SW_A) p1_right = true;
        if (key == HAL_KEY_SW_C) p2_left  = true;
        if (key == HAL_KEY_SW_D) p2_right = true;
        ev_any = true;
    }
    if (state == HAL_KEY_EVENT_UP) {
        if (key == HAL_KEY_SW_B) p1_left  = false;
        if (key == HAL_KEY_SW_A) p1_right = false;
        if (key == HAL_KEY_SW_C) p2_left  = false;
        if (key == HAL_KEY_SW_D) p2_right = false;
    }
}

/* ---- Helpers ---- */

static void reset_ball(Pong *p) {
    p->bx  = (p->server == 0) ? p->p1x + PONG_PADDLE_W / 2
                               : p->p2x + PONG_PADDLE_W / 2;
    p->by  = (p->server == 0) ? PONG_PADDLE_Y1 + 1 : PONG_PADDLE_Y2 - 1;
    p->bdy = (p->server == 0) ? 1 : -1;
    p->bdx = (HAL_get_tick() & 2) ? 1 : -1;
    p->serving = true;
}

static void do_score(Pong *p, uint8_t scorer) {
    if (scorer == 0) p->p1_score++;
    else             p->p2_score++;
    p->server = scorer;   /* scored player serves next */

    if (p->p1_score >= PONG_MAX_SCORE || p->p2_score >= PONG_MAX_SCORE) {
        p->game_over = true;
        p->winner    = scorer;
    } else {
        reset_ball(p);
    }
}

static void render(const Pong *p, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);

    /* Paddles */
    for (int8_t i = 0; i < PONG_PADDLE_W; i++) {
        CBTS_MATRIX_setLedWithColor(display, PONG_PADDLE_Y1, p->p1x + i, P1_COLOR, true);
        CBTS_MATRIX_setLedWithColor(display, PONG_PADDLE_Y2, p->p2x + i, P2_COLOR, true);
    }

    /* Score dots — side columns, away from paddle rows
     * P1 (BLUE): column 0, rows 2..2+score-1
     * P2 (RED):  column 7, rows 13..13-score+1 */
    for (uint8_t s = 0; s < p->p1_score; s++)
        CBTS_MATRIX_setLedWithColor(display, 2 + (int8_t)s, 0, P1_COLOR, true);
    for (uint8_t s = 0; s < p->p2_score; s++)
        CBTS_MATRIX_setLedWithColor(display, 13 - (int8_t)s, PONG_W - 1, P2_COLOR, true);

    /* Ball — blink at 5 Hz while serving */
    bool show_ball = !p->serving || ((HAL_get_tick() / 100) & 1);
    if (show_ball)
        CBTS_MATRIX_setLedWithColor(display, p->by, p->bx, BALL_COLOR, true);

    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void pong_init(Pong *p) {
    p->p1x          = (PONG_W - PONG_PADDLE_W) / 2;
    p->p2x          = (PONG_W - PONG_PADDLE_W) / 2;
    p->p1_score     = 0;
    p->p2_score     = 0;
    p->server       = 0;
    p->game_over    = false;
    p->winner       = 0;
    p->last_ball_tick   = HAL_get_tick();
    p->last_paddle_tick = HAL_get_tick();

    p1_left = p1_right = p2_left = p2_right = false;
    ev_any  = false;
    ev_exit = false;

    reset_ball(p);
    HalKeyConfig(pong_key_cb);
}

bool pong_update(Pong *p, CBTS_MATRIX *display) {
    uint32_t now = HAL_get_tick();

    /* Exit always available via MENU */
    if (ev_exit) return true;

    /* Game over: flash winner color, any other key restarts */
    if (p->game_over) {
        CBTS_MATRIX_clear(display);
        enum LedColor wc = (p->winner == 0) ? P1_COLOR : P2_COLOR;
        if ((now / 300) & 1) {
            for (uint8_t r = 0; r < PONG_H; r++)
                for (uint8_t c = 0; c < PONG_W; c++)
                    CBTS_MATRIX_setLedWithColor(display, r, c, wc, true);
        }
        CBTS_MATRIX_show(display);
        if (ev_any) { ev_any = false; pong_init(p); }
        return false;
    }

    /* Serving: any button launches the ball */
    if (p->serving) {
        render(p, display);
        if (ev_any) {
            ev_any            = false;
            p->serving        = false;
            p->last_ball_tick = now;
        }
        return false;
    }

    /* Move paddles (continuous while held) */
    if (now - p->last_paddle_tick >= PADDLE_MS) {
        p->last_paddle_tick = now;
        if (p1_left  && p->p1x < PONG_W - PONG_PADDLE_W) p->p1x++;
        if (p1_right && p->p1x > 0)                       p->p1x--;
        if (p2_left  && p->p2x < PONG_W - PONG_PADDLE_W) p->p2x++;
        if (p2_right && p->p2x > 0)                       p->p2x--;
    }

    /* Move ball */
    if (now - p->last_ball_tick >= BALL_MS) {
        p->last_ball_tick = now;

        int8_t nx = p->bx + p->bdx;
        int8_t ny = p->by + p->bdy;

        /* Side wall bounces */
        if (nx < 0)       { nx = 0;          p->bdx = -p->bdx; }
        if (nx >= PONG_W) { nx = PONG_W - 1; p->bdx = -p->bdx; }

        /* P1 paddle check (top) */
        if (ny <= PONG_PADDLE_Y1) {
            if (nx >= p->p1x && nx < p->p1x + PONG_PADDLE_W) {
                ny      = PONG_PADDLE_Y1;
                p->bdy  = 1;
                int8_t rel = nx - p->p1x;
                if (rel == 0)                 p->bdx = -1;
                else if (rel == PONG_PADDLE_W - 1) p->bdx =  1;
                /* center hit: keep bdx */
            } else {
                do_score(p, 1);   /* P2 scores */
                render(p, display);
                return false;
            }
        }

        /* P2 paddle check (bottom) */
        if (ny >= PONG_PADDLE_Y2) {
            if (nx >= p->p2x && nx < p->p2x + PONG_PADDLE_W) {
                ny      = PONG_PADDLE_Y2;
                p->bdy  = -1;
                int8_t rel = nx - p->p2x;
                if (rel == 0)                 p->bdx = -1;
                else if (rel == PONG_PADDLE_W - 1) p->bdx =  1;
            } else {
                do_score(p, 0);   /* P1 scores */
                render(p, display);
                return false;
            }
        }

        p->bx = nx;
        p->by = ny;
    }

    render(p, display);
    return false;
}
