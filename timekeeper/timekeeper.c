#include "timekeeper.h"

/* unfortunate globals */
static volatile uint32_t overflow_ctr;
static volatile uint8_t overflow_ctr2;

/* external functions */
void init_timekeeper() {
	overflow_ctr = 0;
	overflow_ctr2 = 0;

	// set registers
}
