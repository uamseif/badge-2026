#ifndef STATS_H
#define STATS_H

#include <stdint.h>

extern uint8_t * const p_game_state;
extern char    * const p_player_name;
extern char    * const p_brand_name;
#define game_state (*p_game_state)

#endif /* STATS_H */
