#ifndef __ccsds_xbee__
#define __ccsds_xbee__

#include <string.h> // needed for memcpy
#include "CCSDS.h"
#include "xbee.h"
#include "add_tlm.h"
/*
also includes :
	uart.h
	stdint.h
	stdbool.h
	avr/io.h
	util/delay.h
	avr/interrupt.h
	stddef.h
*/

// behavioral constants
#define PKT_MAX_LEN 100

// function prototypes
void init_XBee(uint16_t address, uint16_t pan_id);
uint8_t send_tlm_msg(uint16_t send_addr, uint8_t *payload, uint8_t payload_size);
uint8_t send_cmd_msg(uint16_t send_addr, uint8_t fcn_code, uint8_t *payload, uint8_t payload_size);
uint8_t read_msg(uint16_t timeout);
uint8_t read_tlm_msg(uint8_t *data);
uint8_t read_cmd_msg(uint8_t *params, uint8_t *fcn_code);
uint16_t sent_messages();
uint16_t received_messages();

#endif // __ccsds_xbee__
