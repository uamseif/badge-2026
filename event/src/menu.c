#include "menu.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include "font.h"

/* ---- Shared input event flag (any key pressed) ---- */
static bool ev_any_key;

static void any_key_cb(uint16_t key, uint8_t state) {
    (void)key;
    if (state == HAL_KEY_EVENT_DOWN) ev_any_key = true;
}

/* ---- Game registry ---- */

#define MENU_GAME_COUNT  3

static const char * const GAME_NAMES[MENU_GAME_COUNT]  = { "TETRIS", "PONG",   "SNAKE" };
static const enum LedColor GAME_COLORS[MENU_GAME_COUNT] = { CYAN,     YELLOW,   GREEN   };
static const AppState      GAME_STATES[MENU_GAME_COUNT] = { APP_TETRIS, APP_PONG, APP_SNAKE };

/* ---- Text rendering (horizontal orientation) ----
 *
 * Uses the "row" axis (0-15) as horizontal and "column" axis (0-7) as
 * vertical, matching the official (landscape) PCB orientation.
 * First arg of setLedWithColor = horizontal position (0-15).
 * Second arg                   = vertical position (0-7).
 */

static int font_index(char c) {
    if (c == ' ')              return 0;
    if (c >= '0' && c <= '9') return 1 + (c - '0');
    if (c >= 'A' && c <= 'Z') return 11 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 37 + (c - 'a');
    return 0;
}

static void draw_char(CBTS_MATRIX *display, char c, int x, enum LedColor color) {
    int idx = font_index(c);
    for (int col = 0; col < 5; col++) {
        uint8_t line = font5x7[idx][col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row))
                CBTS_MATRIX_setLedWithColor(display, x + col, row + 1, color, true);
        }
    }
}

static void draw_text(CBTS_MATRIX *display, const char *text, int offset,
                      enum LedColor color) {
    int x = 16 - offset;
    while (*text) {
        draw_char(display, *text, x, color);
        x += 6;
        text++;
    }
}

static int text_scroll_width(const char *text) {
    int len = 0;
    while (*text++) len++;
    return len * 6 + 16;  /* extra 16 so text fully exits left before reset */
}

/* ---- Marquee ---- */

static int      marquee_scroll;
static int      marquee_width;
static uint8_t  marquee_color_idx;
static uint32_t marquee_last_tick;

void marquee_init(void) {
    marquee_scroll    = 0;
    marquee_width     = text_scroll_width("Cibertracks");
    marquee_color_idx = 0;
    marquee_last_tick = HAL_get_tick();
    ev_any_key        = false;
    HalKeyConfig(any_key_cb);
}

/* Returns true when any key is pressed (transition to menu). */
bool marquee_update(CBTS_MATRIX *display) {
    if (ev_any_key) return true;

    uint32_t now = HAL_get_tick();
    if (now - marquee_last_tick < 50) return false;
    marquee_last_tick = now;

    static const enum LedColor rainbow[] = {
        RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, WHITE
    };

    CBTS_MATRIX_clear(display);
    draw_text(display, "Cibertracks",
              marquee_scroll,
              rainbow[marquee_color_idx % 7]);
    CBTS_MATRIX_show(display);

    marquee_scroll++;
    if (marquee_scroll > marquee_width) {
        marquee_scroll = 0;
        marquee_color_idx++;
    }
    return false;
}

/* ---- Menu ---- */

static uint8_t  menu_selected;
static int      menu_scroll;
static int      menu_name_width;
static uint32_t menu_last_tick;
static uint8_t  menu_ev_nav;   /* navigation events: left / right */

#define MENU_EV_LEFT  0x01
#define MENU_EV_RIGHT 0x02
#define MENU_EV_SEL   0x04

static void menu_key_cb(uint16_t key, uint8_t state) {
    if (state != HAL_KEY_EVENT_DOWN) return;
    if (key == HAL_KEY_SW_B) menu_ev_nav |= MENU_EV_LEFT;
    if (key == HAL_KEY_SW_C) menu_ev_nav |= MENU_EV_RIGHT;
    /* Any other key = select */
    if (key != HAL_KEY_SW_B && key != HAL_KEY_SW_C) menu_ev_nav |= MENU_EV_SEL;
}

void menu_init(void) {
    menu_selected   = 0;
    menu_scroll     = 0;
    menu_name_width = text_scroll_width(GAME_NAMES[0]);
    menu_last_tick  = HAL_get_tick();
    menu_ev_nav     = 0;
    HalKeyConfig(menu_key_cb);
}

/* Returns the next AppState. Scrolls the selected game name; any button
 * starts the game. Left/right (C/D) navigate between games. */
AppState menu_update(CBTS_MATRIX *display) {
    uint8_t ev  = menu_ev_nav;
    menu_ev_nav = 0;

    /* Navigate between games (wraps around) */
    if (ev & MENU_EV_LEFT) {
        menu_selected   = (menu_selected + MENU_GAME_COUNT - 1) % MENU_GAME_COUNT;
        menu_scroll     = 0;
        menu_name_width = text_scroll_width(GAME_NAMES[menu_selected]);
    }
    if (ev & MENU_EV_RIGHT) {
        menu_selected   = (menu_selected + 1) % MENU_GAME_COUNT;
        menu_scroll     = 0;
        menu_name_width = text_scroll_width(GAME_NAMES[menu_selected]);
    }

    /* Select */
    if (ev & MENU_EV_SEL) {
        menu_scroll = 0;
        return GAME_STATES[menu_selected];
    }

    uint32_t now = HAL_get_tick();
    if (now - menu_last_tick < 50) return APP_MENU;
    menu_last_tick = now;

    CBTS_MATRIX_clear(display);
    draw_text(display, GAME_NAMES[menu_selected],
              menu_scroll,
              GAME_COLORS[menu_selected]);
    CBTS_MATRIX_show(display);

    menu_scroll++;
    if (menu_scroll > menu_name_width)
        menu_scroll = 0;

    return APP_MENU;
}
