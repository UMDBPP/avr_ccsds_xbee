#ifndef __timekeeper__
#define __timekeeper__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/* Function Prototypes */
void init_timekeeper();
uint32_t get_ms();
void delay(uint32_t ms); // input time in ms
void reset_timekeeper();

#endif  // __timekeeper__
