#include "space_inv.h"
#include "gpio_hal.h"
#include "systick_hal.h"

#define PLAYER_ROW    14
#define PLAYER_MS     80
#define BULLET_MS     50
#define ABUL_MS       150
#define ALIEN_MS_MAX  500   /* ms per step when grid is full */
#define ALIEN_MS_MIN  80    /* ms per step when last alien remains */

static const enum LedColor ALIEN_COLORS[SI_ROWS] = { RED, YELLOW, GREEN };

/* ---- Input ---- */
static bool held_left, held_right, ev_shoot, ev_exit, ev_any;

static void si_key_cb(uint16_t key, uint8_t state) {
    if (key == HAL_KEY_SW_MENU) {
        if (state == HAL_KEY_EVENT_LONG) ev_exit = true;
        return;
    }
    if (state == HAL_KEY_EVENT_DOWN) {
        ev_any = true;
        if      (key == HAL_KEY_SW_C) held_left  = true;
        else if (key == HAL_KEY_SW_D) held_right = true;
        else                          ev_shoot   = true;
    } else if (state == HAL_KEY_EVENT_UP) {
        if (key == HAL_KEY_SW_C) held_left  = false;
        if (key == HAL_KEY_SW_D) held_right = false;
    }
}

/* ---- RNG ---- */
static uint8_t rng_next(uint8_t *s) {
    *s ^= (uint8_t)(*s << 7);
    *s ^= (uint8_t)(*s >> 5);
    *s ^= (uint8_t)(*s << 3);
    return *s;
}

/* ---- Helpers ---- */
static uint8_t count_alive(const SpaceInvaders *si) {
    uint8_t n = 0;
    for (int r = 0; r < SI_ROWS; r++) {
        uint8_t b = si->aliens[r];
        while (b) { n++; b &= (uint8_t)(b - 1); }  /* Kernighan bit count */
    }
    return n;
}

static int8_t leftmost_alive(const SpaceInvaders *si) {
    for (int c = 0; c < SI_COLS; c++)
        for (int r = 0; r < SI_ROWS; r++)
            if (si->aliens[r] & (1 << c)) return (int8_t)c;
    return 0;
}

static int8_t rightmost_alive(const SpaceInvaders *si) {
    for (int c = SI_COLS - 1; c >= 0; c--)
        for (int r = 0; r < SI_ROWS; r++)
            if (si->aliens[r] & (1 << c)) return (int8_t)c;
    return (int8_t)(SI_COLS - 1);
}

static int8_t lowest_alive_row(const SpaceInvaders *si) {
    for (int r = SI_ROWS - 1; r >= 0; r--)
        if (si->aliens[r]) return (int8_t)r;
    return 0;
}

static uint32_t alien_step_ms(const SpaceInvaders *si) {
    uint8_t alive = count_alive(si);
    if (alive == 0) return ALIEN_MS_MIN;
    /* Decrease base speed each level */
    uint32_t base = (uint32_t)ALIEN_MS_MAX;
    if (si->level > 0) base = base > (uint32_t)(si->level * 60) ? base - (uint32_t)(si->level * 60) : ALIEN_MS_MIN;
    return (uint32_t)ALIEN_MS_MIN + (base - ALIEN_MS_MIN) * alive / SI_TOTAL;
}

static uint32_t shoot_interval(const SpaceInvaders *si) {
    uint32_t ms = 2000;
    uint32_t dec = (uint32_t)(si->level * 200);
    ms = ms > dec ? ms - dec : 600;
    return ms < 600 ? 600 : ms;
}

static void si_reset_level(SpaceInvaders *si) {
    for (int r = 0; r < SI_ROWS; r++)
        si->aliens[r] = (uint8_t)((1u << SI_COLS) - 1);
    si->alien_x  = 1;
    si->alien_y  = 0;
    si->alien_dx = 1;
    si->bullet_r = si->abul_r = -1;
    si->victory  = false;
    si->game_over = false;

    uint32_t t = HAL_get_tick();
    si->alien_last_tick  = t;
    si->bullet_last_tick = t;
    si->abul_last_tick   = t;
    si->shoot_last_tick  = t;
    si->player_last_tick = t;

    si->alien_ms = alien_step_ms(si);
}

static void render(const SpaceInvaders *si, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);

    /* Aliens */
    for (int r = 0; r < SI_ROWS; r++) {
        for (int c = 0; c < SI_COLS; c++) {
            if (si->aliens[r] & (1 << c)) {
                int8_t row = (int8_t)(si->alien_y + r);
                int8_t col = (int8_t)(si->alien_x + c);
                if (row >= 0 && row < PLAYER_ROW && col >= 0 && col < 8)
                    CBTS_MATRIX_setLedWithColor(display, row, col, ALIEN_COLORS[r], true);
            }
        }
    }

    /* Player */
    CBTS_MATRIX_setLedWithColor(display, PLAYER_ROW, si->player_x, CYAN, true);

    /* Player bullet */
    if (si->bullet_r >= 0)
        CBTS_MATRIX_setLedWithColor(display, si->bullet_r, si->bullet_c, WHITE, true);

    /* Alien bullet */
    if (si->abul_r >= 0 && si->abul_r < PLAYER_ROW)
        CBTS_MATRIX_setLedWithColor(display, si->abul_r, si->abul_c, MAGENTA, true);

    /* Lives (cyan dots at row 15) */
    for (uint8_t i = 0; i < si->lives; i++)
        CBTS_MATRIX_setLedWithColor(display, 15, i * 2, CYAN, true);

    CBTS_MATRIX_show(display);
}

/* ---- Public API ---- */

void si_init(SpaceInvaders *si) {
    si->lives = 3;
    si->score = 0;
    si->level = 0;
    si->rng   = (uint8_t)(HAL_get_tick() | 1);
    si->player_x = 3;

    held_left = held_right = ev_shoot = ev_exit = ev_any = false;
    si_reset_level(si);
    HalKeyConfig(si_key_cb);
}

bool si_update(SpaceInvaders *si, CBTS_MATRIX *display) {
    if (ev_exit) return true;

    uint32_t now = HAL_get_tick();

    /* Game over: flash player column red, any key restarts */
    if (si->game_over) {
        CBTS_MATRIX_clear(display);
        if ((now / 300) & 1)
            for (int r = 0; r <= PLAYER_ROW; r++)
                CBTS_MATRIX_setLedWithColor(display, r, si->player_x, RED, true);
        CBTS_MATRIX_show(display);
        if (ev_any) { ev_any = false; si_init(si); }
        return false;
    }

    /* Victory: rainbow flash, then advance to next level */
    if (si->victory) {
        static const enum LedColor vc[] = { RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA };
        CBTS_MATRIX_clear(display);
        enum LedColor col = vc[(now / 80) % 6];
        for (int r = 0; r < PLAYER_ROW; r++)
            for (int c = 0; c < 8; c++)
                CBTS_MATRIX_setLedWithColor(display, r, c, col, true);
        CBTS_MATRIX_show(display);
        if (now - si->victory_tick > 1500) {
            si->level++;
            si_reset_level(si);
        }
        return false;
    }

    /* ---- Player movement ---- */
    if (now - si->player_last_tick >= PLAYER_MS) {
        si->player_last_tick = now;
        if (held_left  && si->player_x < 7) si->player_x++;
        if (held_right && si->player_x > 0) si->player_x--;
    }

    /* ---- Player shoot ---- */
    if (ev_shoot) {
        ev_shoot = false;
        if (si->bullet_r < 0) {
            si->bullet_r = PLAYER_ROW - 1;
            si->bullet_c = si->player_x;
            si->bullet_last_tick = now;
        }
    }

    /* ---- Player bullet movement + alien collision ---- */
    if (si->bullet_r >= 0 && now - si->bullet_last_tick >= BULLET_MS) {
        si->bullet_last_tick = now;
        si->bullet_r--;

        if (si->bullet_r < 0) {
            si->bullet_r = -1;
        } else {
            int8_t gc = (int8_t)(si->bullet_c - si->alien_x);
            if (gc >= 0 && gc < SI_COLS) {
                for (int r = 0; r < SI_ROWS; r++) {
                    if (si->bullet_r == si->alien_y + r &&
                        (si->aliens[r] & (1 << gc))) {
                        si->aliens[r] &= ~(uint8_t)(1u << gc);
                        si->score++;
                        si->bullet_r = -1;
                        si->alien_ms = alien_step_ms(si);
                        if (count_alive(si) == 0) {
                            si->victory      = true;
                            si->victory_tick = now;
                        }
                        break;
                    }
                }
            }
        }
    }

    /* ---- Alien movement ---- */
    if (now - si->alien_last_tick >= si->alien_ms) {
        si->alien_last_tick = now;

        int8_t left  = leftmost_alive(si);
        int8_t right = rightmost_alive(si);
        int8_t new_x = (int8_t)(si->alien_x + si->alien_dx);

        if (new_x + right > 7 || new_x + left < 0) {
            /* Hit wall: reverse direction and drop one row */
            si->alien_dx = (int8_t)-si->alien_dx;
            si->alien_y++;
            if (si->alien_y + lowest_alive_row(si) >= PLAYER_ROW - 1)
                si->game_over = true;
        } else {
            si->alien_x = new_x;
        }
    }

    /* ---- Alien bullet movement + player collision ---- */
    if (si->abul_r >= 0 && now - si->abul_last_tick >= ABUL_MS) {
        si->abul_last_tick = now;
        si->abul_r++;
        if (si->abul_r >= PLAYER_ROW) {
            if (si->abul_r == PLAYER_ROW && si->abul_c == si->player_x) {
                si->lives--;
                if (si->lives == 0) si->game_over = true;
            }
            si->abul_r = -1;
        }
    }

    /* ---- Alien shooting ---- */
    if (si->abul_r < 0 && now - si->shoot_last_tick >= shoot_interval(si)) {
        si->shoot_last_tick = now;
        uint8_t col = (uint8_t)(rng_next(&si->rng) % SI_COLS);
        int8_t  row_found = -1;
        for (int r = SI_ROWS - 1; r >= 0; r--) {
            if (si->aliens[r] & (1 << col)) { row_found = (int8_t)r; break; }
        }
        if (row_found >= 0) {
            si->abul_r = (int8_t)(si->alien_y + row_found + 1);
            si->abul_c = (int8_t)(si->alien_x + col);
            si->abul_last_tick = now;
        }
    }

    render(si, display);
    return false;
}
