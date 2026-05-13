#ifndef STATS_H
#define STATS_H

#include <stdint.h>

/* game_state is the active AppState. It lives inside _s (stats.c) right
 * after achievement_log[9], so writing achievement_log[9] aliases it.
 * Access via the macro so all callers spell it the same way. */
extern uint8_t * const p_game_state;
#define game_state (*p_game_state)

void stats_record(uint16_t score);

#endif /* STATS_H */
