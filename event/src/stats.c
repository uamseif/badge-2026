#include "stats.h"

/* The struct guarantees that achievement_log[0..3] immediately precedes
 * gs in memory. stats_record() uses 'score' as an index with no
 * upper-bound check: when score == 4 the write lands on gs (= game_state). */
static struct {
    uint8_t achievement_log[4];
    uint8_t gs;
} _s;

uint8_t * const p_game_state = &_s.gs;

void stats_record(uint16_t score) {
    _s.achievement_log[score]++;
}
