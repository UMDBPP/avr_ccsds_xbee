#include "timekeeper.h"

/* Register Definitions */
// Timer/Counter 0 for all chipsets
#if defined(__AVR_ATmega328P__)
	#define TC0_OVERFLOW_VECT TIMER0_OVF_vect
	#define TC0_INT_REG TIMSK0
	#define TC0_INT_EN TOIE0
	#define PRESCALER_BANK TCCR0B
	#define CS_BIT0 CS00
	#define CS_BIT1 CS01
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega640__)
	#define TC0_OVERFLOW_VECT TIMER0_OVF_vect
	#define TC0_INT_REG TIMSK0
	#define TC0_INT_EN TOIE0
	#define PRESCALER_BANK TCCR0B
	#define CS_BIT0 CS00
	#define CS_BIT1 CS01
#elif defined(__AVR_ATtiny828__)
	#define TC0_OVERFLOW_VECT TIMER0_OVF_vect
	#define TC0_INT_REG TIMSK0
	#define TC0_INT_EN TOIE0
	#define PRESCALER_BANK TCCR0B
	#define CS_BIT0 CS00
	#define CS_BIT1 CS01
#elif defined(__AVR_ATtiny841__)
	#define TC0_OVERFLOW_VECT TIMER0_OVF_vect
	#define TC0_INT_REG TIMSK0
	#define TC0_INT_EN TOIE0
	#define PRESCALER_BANK TCCR0B
	#define CS_BIT0 CS00
	#define CS_BIT1 CS01
#elif defined(__AVR_ATmega88PA__) || defined(__AVR_ATmega168PA__)
	#define TC0_OVERFLOW_VECT TIMER0_OVF_vect
	#define TC0_INT_REG TIMSK0
	#define TC0_INT_EN TOIE0
	#define PRESCALER_BANK TCCR0B
	#define CS_BIT0 CS00
	#define CS_BIT1 CS01
#elif defined(__AVR_ATmega169PA__) || defined(__AVR_ATmega329PA__)
	#define TC0_OVERFLOW_VECT TIMER0_OVF_vect
	#define TC0_INT_REG TIMSK0
	#define TC0_INT_EN TOIE0
	#define PRESCALER_BANK TCCR0A
	#define CS_BIT0 CS00
	#define CS_BIT1 CS01
#elif defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644P__)
	#define TC0_OVERFLOW_VECT TIMER0_OVF_vect
	#define TC0_INT_REG TIMSK0
	#define TC0_INT_EN TOIE0
	#define PRESCALER_BANK TCCR0B
	#define CS_BIT0 CS00
	#define CS_BIT1 CS01
#endif

/* Behavioral Constants */
#define uS_PER_OVERFLOW ((64*256)/(F_CPU/1000000L))
#define MS_INC (uS_PER_OVERFLOW/1000)
#define MS_FRAC_INC (((uS_PER_OVERFLOW) % 1000) >> 3)

/* Globals */
static volatile uint32_t overflow_ctr; // represents ms
static volatile uint8_t overflow_ctr_frac;

/* External Functions */
void init_timekeeper() {
	overflow_ctr = 0;
	overflow_ctr_frac = 0;

	// set registers
	PRESCALER_BANK |= (1 << CS_BIT0) | (1 << CS_BIT1); // set prescalar 64
	TC0_INT_REG |= (1 << TC0_INT_EN); // enable timer overflow interrupt

	sei(); // enable global interrupts
}

uint32_t get_ms() {
	uint8_t old_SREG = SREG;
	cli();

	uint32_t local_ms = overflow_ctr;

	SREG = old_SREG;
	return local_ms;
}

void delay(uint32_t delay_ms) {
	uint32_t delay_check = get_ms() + delay_ms;
	while (get_ms() <= delay_check); // do nothing till times match
}

void reset_timekeeper() {
	uint8_t old_SREG = SREG;
	cli();

	overflow_ctr = 0;
	overflow_ctr_frac = 0;

	SREG = old_SREG;
}

ISR(TC0_OVERFLOW_VECT) { // interrupt service routine for tracking time
	overflow_ctr += MS_INC;
	overflow_ctr_frac += MS_FRAC_INC;
	if (overflow_ctr_frac >= 125) {
		overflow_ctr_frac -= 125; // In case we go over, keep the fraction
		overflow_ctr += 1;
	}
}
