#ifndef __uart_xbee__
#define __uart_xbee__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

/* function prototypes */
void init_UART();
void send_message(uint8_t *array, uint8_t size);
uint8_t read_rx();
uint16_t get_payload_length();
void cpy_data(uint8_t *array);

#endif // __uart_xbee__
