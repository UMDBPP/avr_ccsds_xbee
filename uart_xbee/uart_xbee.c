#include "uart_xbee.h"

/* Register Names */
#if defined(__AVR_ATmega328P__)
	// RX = PD0, TX = PD1
	#define BAUD_H UBRR0H
	#define BAUD_L UBRR0L
	#define BANKB UCSR0B
	#define BANKC UCSR0C
	#define EN_RX RXEN0
	#define EN_TX TXEN0
	#define BANKA UCSR0A
	#define TX_CLEAR UDRE0
	#define DATA_REG UDR0
	#define RX_INT_EN RXCIE0
	#define RX_ISR USART_RX_vect
	#define FRAME_BITS UCSZ00
#elif defined(__AVR_ATmega2560__)
	// usart3
	// RX = PJ0, TX = PJ1
	#define BAUD_H UBRR3H
	#define BAUD_L UBRR3L
	#define BANKB UCSR3B
	#define BANKC UCSR3C
	#define EN_RX RXEN3
	#define EN_TX TXEN3
	#define BANKA UCSR3A
	#define TX_CLEAR UDRE3
	#define DATA_REG UDR3
	#define RX_INT_EN RXCIE3
	#define RX_ISR USART3_RX_vect
	#define FRAME_BITS UCSZ30
#elif defined(__AVR_ATtiny4313__) || defined(__AVR_ATtiny2313A__)
// NOTE: THIS CHIP IS NOT USABLE DUE TO THE LACK OF SUFFICIENT MEMORY!!!
	// RX = PD0, TX = PD1
	#define BAUD_H UBRRH
	#define BAUD_L UBRRL
	#define BANKB UCSRB
	#define BANKC UCSRC
	#define EN_RX RXEN
	#define EN_TX TXEN
	#define BANKA UCSRA
	#define TX_CLEAR UDRE
	#define DATA_REG UDR
	#define RX_INT_EN RXCIE
	#define RX_ISR USART0_RX_vect
	#define FRAME_BITS UCSZ0
#elif defined(__AVR_ATtiny441__) || defined(__AVR_ATtiny841__)
	// RX = PA4, TX = PA5
	#define BAUD_H UBRR1H
	#define BAUD_L UBRR1L
	#define BANKB UCSR1B
	#define BANKC UCSR1C
	#define EN_RX RXEN1
	#define EN_TX TXEN1
	#define BANKA UCSR1A
	#define TX_CLEAR UDRE1
	#define DATA_REG UDR1
	#define RX_INT_EN RXCIE1
	#define RX_ISR USART1_RX_vect
	#define FRAME_BITS UCSZ10
#endif

/* Program Constants */
#define BAUD 9600
#define UART_BAUD (((F_CPU/16)/BAUD)-1)
#define BUFFER_SIZE 256

/* unfortunate globals */
static uint8_t rx_buffer[BUFFER_SIZE]; // buffer for holding values read in on UART
static uint8_t rx_pos; // value for iterating through buffer when adding to it
static uint8_t read_position; // value for iterating through buffer for parsing it
static uint16_t payload_length;

/* private helper functions */
static uint16_t get_length(uint8_t *low, uint8_t *high);

void init_UART() {
/* Initialization function for the UART on the XBEE ports */
	// set baud rate
	BAUD_H = (uint8_t) (UART_BAUD >> 8);
	BAUD_L = (uint8_t) UART_BAUD;
	// enable tx and rx
	BANKB = (1 << EN_RX) | (1 << EN_TX);
	// set frame format - 8 bits char length, 1 stop
	BANKC = (3 << FRAME_BITS);

	BANKB |= (1 << RX_INT_EN); // enable RX interrupt
	sei(); // enable global interrupt

	// initialize variables
	rx_pos = 0;
	read_position = 0;
	payload_length = 0;
}

void send_message(uint8_t *array, uint8_t size) {
/* this function will send the contents of the array of given size to the attached
   XBee, which should broadcast it */
	for (uint8_t i = 0; i < size; i++) {
		while (!(BANKA & (1 << TX_CLEAR))); // wait for register to clear
		DATA_REG = *(array+i); // add next value to the register
	}
}

uint16_t get_length(uint8_t *lsb, uint8_t *msb) {
/* get the length of the data by reconstructing the high bit and low bit. This particular
   function is dependent on the structure of XBee frames. helper method */
	return (uint16_t)(((*msb) << 8) | ((*lsb) & (uint8_t)0xFF));
}

uint16_t get_payload_length() {
	return payload_length;
}

uint8_t read_rx() {
/* checks for incoming bytes, returns the XBee frame type */
	for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
		if ((uint8_t)rx_buffer[i] == 0x7E) { // start delimeter for XBee frames is 0x7E
			//_delay_ms(10);
			rx_buffer[i] = 0; // reset, so we don't read it twice
			// the code with ternary operators instead of modulo is there because avr does not have a division piece in the alu, which makes it slow. However, it is fast with powers of two because it becomes a bitwise and. If the buffer size is not a power of two, use the ternary operator instead.
			uint8_t *msb = rx_buffer + ((i + 1) % 256); // after start delimeter, the next two bytes are for length and the one after that is for frame type
			//uint8_t *msb = rx_buffer + (((i + 1) < 256) ? (i + 1) : (i + 1 - 256));
			uint8_t *lsb = rx_buffer + ((i + 2) % 256);
			//uint8_t *lsb = rx_buffer + (((i + 2) < 256) ? (i + 2) : (i + 2 - 256));
			read_position = ((i + 3) % 256); // set the read position to where the data starts
			//read_position = ((i + 3) < 256) ? (i + 3) : (i + 3 - 256);
			payload_length = get_length(lsb, msb);
			return *(rx_buffer + ((i + 4) % 256));
			//return = *(rx_buffer + (((i + 4) < 256) ? (i + 4) : (i + 4 - 256)));
		}
	}
	return 0;
}

void cpy_data(uint8_t *array) {
/* function to copy the contents of a XBee frame to the provided array */
	for (uint8_t i = 0; i < payload_length; i++) {
		*(array+i) = rx_buffer[(read_position + i) % 256];
		//*(array+i) = rx_buffer[((read_position + i) < 256) ? (read_position + i) : (read_position + i - 256)];
	}
}

ISR(RX_ISR) {
/* ISR used for adding newly read values into the buffer */
	cli(); // disable interrupts
	rx_buffer[rx_pos] = (uint8_t) DATA_REG; // add read value to the buffer, and then update the buffer location
	if (rx_pos < 255) { // division is expensive compared to this
		rx_pos++;
	}
	else {
		rx_pos = 0;
	}
	sei(); // enable interrupts
}
