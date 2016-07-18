#include "timekeeper.h"

/* Register Definitions */
// Timer/Counter 1 for all chipsets
#if defined(__AVR_ATmega328P__)
	
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega640__)
	
#elif defined(__AVR_ATtiny828__)
	
#elif defined(__AVR_ATtiny841__)
	
#elif defined(__AVR_ATmega88PA__) || defined(__AVR_ATmega168PA__)
	
#elif defined(__AVR_ATmega169PA__) || defined(__AVR_ATmega329PA__)
	
#elif defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644PA__)
	
#endif

/* Unfortunate Globals */
static volatile uint32_t overflow_ctr;
static volatile uint8_t overflow_ctr2;

/* External Functions */
void init_timekeeper() {
	overflow_ctr = 0;
	overflow_ctr2 = 0;

	// set registers
}

uint32_t get_ms() {

}

void delay(uint32_t ms) {

}
