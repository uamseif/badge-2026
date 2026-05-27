#include "stats.h"

static struct {
    char    name[6];
    uint8_t gs;
    char    brand[12];
} _s = {
    .name  = {' ', ' ', ' ', ' ', ' ', ' '},
    .brand = "C1b3rtr4cks"
};

uint8_t * const p_game_state  = &_s.gs;
char    * const p_player_name = _s.name;
char    * const p_brand_name  = _s.brand;
