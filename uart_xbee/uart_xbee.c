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
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__)
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
#elif defined(__AVR_ATtiny828__)
	// RX = PC2, TX = PC3
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
	#define RX_ISR USART_RX_vect
	#define FRAME_BITS UCSZ0
#elif defined(__AVR_ATtiny841__)
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
#elif defined(__AVR_ATmega88PA__) || defined(__AVR_ATmega168PA__)
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
#elif defined(__AVR_ATmega169PA__) || defined(__AVR_ATmega329PA__)
	// RX = PE0, TX = PE1
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
	#define RX_ISR USART0_RX_vect
	#define FRAME_BITS UCSZ00
#elif defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644P__)
	// RX = PD2, TX = PD3
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
// keep buffer size power of 2, or there will be perfomance hit since there is no division in ALU

/* Globals */
static uint8_t rx_buffer[BUFFER_SIZE]; // buffer for holding values read in on UART
static uint8_t rx_pos; // value for iterating through buffer when adding to it
static uint8_t read_position; // value for iterating through buffer for parsing it
static uint16_t payload_length;

/* Private Helper Functions */
static uint16_t get_length(uint8_t *lsb, uint8_t *msb) {
/* get the length of the data by reconstructing the high bit and low bit. This particular
   function is dependent on the structure of XBee frames. helper method */
	return (uint16_t)(((*msb) << 8) | ((*lsb) & (uint8_t)0xFF));
}

/* Externally Visible */
void init_UART() {
/* Initialization function for the UART on the XBEE ports */
	init_timekeeper(); // starts the clock and enables global interrupts

	// set baud rate
	BAUD_H = (uint8_t) (UART_BAUD >> 8);
	BAUD_L = (uint8_t) UART_BAUD;
	// enable tx and rx
	BANKB = (1 << EN_RX) | (1 << EN_TX);
	// set frame format - 8 bits char length, 1 stop
	BANKC = (3 << FRAME_BITS);

	BANKB |= (1 << RX_INT_EN); // enable RX interrupt

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

uint16_t get_payload_length() {
	return payload_length;
}

uint8_t read_rx() {
/* checks for incoming bytes, returns the XBee frame type */
	for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
		if ((uint8_t)rx_buffer[i] == 0x7E) { // start delimeter for XBee frames is 0x7E
			//_delay_ms(10);
			rx_buffer[i] = 0; // reset, so we don't read it twice

			uint8_t *msb = rx_buffer + ((i + 1) % BUFFER_SIZE); // after start delimeter, the next two bytes are for length and the one after that is for frame type
			uint8_t *lsb = rx_buffer + ((i + 2) % BUFFER_SIZE);
			read_position = ((i + 3) % BUFFER_SIZE); // set the read position to where the data starts

			payload_length = get_length(lsb, msb);
			return *(rx_buffer + ((i + 4) % BUFFER_SIZE));
		}
	}
	return 0;
}

void cpy_data(uint8_t *array) {
/* function to copy the contents of a XBee frame to the provided array */
	for (uint8_t i = 0; i < payload_length; i++) {
		*(array+i) = rx_buffer[(read_position + i) % BUFFER_SIZE];
	}
}

/* Interrupt Service Routines */
ISR(RX_ISR) {
/* ISR used for adding newly read values into the buffer */
	uint8_t old_SREG = SREG; // store state of status register

	cli(); // disable interrupts
	rx_buffer[rx_pos] = (uint8_t) DATA_REG; // add read value to the buffer, and then update the buffer location
	if (rx_pos >= BUFFER_SIZE) {
		rx_pos = 0;
	}
	else {
		rx_pos++;
	}

	SREG = old_SREG; // revert state of status register
}
