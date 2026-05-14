#include "name_editor.h"
#include "gpio_hal.h"
#include "systick_hal.h"
#include "stats.h"
#include "font3x5.h"

#define NAME_LEN  6

static bool   ev_exit;
static int8_t ev_glyph;
static int8_t ev_cursor;
static bool   stay_in_editor;
static bool   reset_cursor;

static void ne_key_cb(uint16_t key, uint8_t state) {
    if (state == HAL_KEY_EVENT_DOWN) {
        if (key == HAL_KEY_SW_A) ev_glyph = -1;
        if (key == HAL_KEY_SW_D) ev_glyph = +1;
        if (key == HAL_KEY_SW_B) ev_cursor = -1;
        if (key == HAL_KEY_SW_C) ev_cursor = +1;
        if (key == HAL_KEY_SW_MENU) stay_in_editor = false;
    } else if (state == HAL_KEY_EVENT_LONG) {
        if (!stay_in_editor) reset_cursor = true;
        ev_exit = true;
    }
}

static int ascii_to_glyph(char c) {
    if (c == ' ')              return 0;
    if (c >= '0' && c <= '9') return 1 + (c - '0');
    if (c >= 'A' && c <= 'Z') return 11 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 37 + (c - 'a');
    return 0;
}

static void draw_glyph(CBTS_MATRIX *display, char c, int x, enum LedColor color) {
    int idx = ascii_to_glyph(c);
    for (int col = 0; col < 3; col++) {
        uint8_t line = font3x5[idx][col];
        for (int row = 0; row < 5; row++) {
            if (line & (1 << row))
                CBTS_MATRIX_setLedWithColor(display, x + col, row + 2, color, true);
        }
    }
}

static void render(const NameEditor *ne, CBTS_MATRIX *display) {
    CBTS_MATRIX_clear(display);

    if (ne->cursor > 0)
        draw_glyph(display, p_player_name[ne->cursor - 1], 2, BLUE);

    draw_glyph(display, p_player_name[ne->cursor], 6, WHITE);

    if (ne->cursor < NAME_LEN - 1)
        draw_glyph(display, p_player_name[ne->cursor + 1], 10, BLUE);

    /* Position indicator: 6 dots at bottom row */
    for (int i = 0; i < NAME_LEN; i++)
        CBTS_MATRIX_setLedWithColor(display, i * 2 + 2, 7,
            (i == ne->cursor) ? WHITE : BLUE, true);

    CBTS_MATRIX_show(display);
}

void name_editor_init(NameEditor *ne) {
    int8_t current_cursor = -1;
    for (uint8_t i = 0; i < NAME_LEN; i++) {
        if (p_player_name[i] == ' ') {
            current_cursor = i;
            break;
        }
    }
    if (current_cursor >= 0 ) {
        ne->cursor = current_cursor;
    } else if (reset_cursor) {
        ne->cursor = 0;
    }

    stay_in_editor = true;
    reset_cursor = false;
    ev_exit   = false;
    ev_glyph  = 0;
    ev_cursor = 0;
    HalKeyConfig(ne_key_cb);
}

bool name_editor_update(NameEditor *ne, CBTS_MATRIX *display) {
    if (ev_exit) return true;

    int8_t g = ev_glyph;  ev_glyph  = 0;
    int8_t c = ev_cursor; ev_cursor = 0;

    if (g) {
        char cv = p_player_name[ne->cursor];
        if (g > 0) {
            /* cycle forward: A→...→Z→0→...→9→A */
            if      (cv >= 'A' && cv < 'Z') cv++;
            else if (cv == 'Z')             cv = '0';
            else if (cv >= '0' && cv < '9') cv++;
            else if (cv == '9')             cv = 'A';
            else                            cv = 'A';
        } else {
            /* cycle backward: A→9→...→0→Z→...→A */
            if      (cv > 'A' && cv <= 'Z') cv--;
            else if (cv == 'A')             cv = '9';
            else if (cv > '0' && cv <= '9') cv--;
            else if (cv == '0')             cv = 'Z';
            else                            cv = '9';
        }
        p_player_name[ne->cursor] = cv;
    }

    if (c) {
        int cv = (int)ne->cursor + c;
        if (cv < 0)         cv = 0;
        if (cv >= NAME_LEN) cv = NAME_LEN - 1;
        ne->cursor = (uint8_t)cv;
    }

    render(ne, display);
    return false;
}
