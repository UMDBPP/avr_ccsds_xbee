#include "xbee.h"

/* Private Helper Functions */
static void add_checksum(uint8_t *array, uint8_t size) {
/* The checksum as defined by Digi is to add all of the byte values in
   the data field of the frame, take the lowest 8 bits, and subtract that
   from 255 */
	uint16_t value = 0;
	for (uint8_t i = 3; i < (size-1); i++) { // iterate through array and add values
		value += *(array+i);
	}
	value = ((uint8_t)0xFF & value); // take lowest 8 bits

	*(array+(size-1)) = (0xFF - (uint8_t)value); // add checksum to the appropriate place in array
}

/* Externally Visible */
/* For Data_Frame Type (to send data to other XBee's) */
void data_add_target(struct data_frame *packet, uint16_t target) {
/* put the 16 bit integer target into it's respective high and low byte fields */
	packet -> dest_lsb = (uint8_t)((uint8_t)0xFF & target);
	packet -> dest_msb = (uint8_t)((target >> 8) & (uint8_t)0xFF);
}

void data_add_frame_id(struct data_frame *packet, uint8_t id) {
	packet -> frame_id = id;
}

void data_add_options(struct data_frame *packet, uint8_t option_choice) {
	packet -> options = option_choice;
}

void data_send_frame(struct data_frame *packet, uint8_t *data_arr, uint8_t size) {
/* Construct the rest of the packet, and send it to the chosen target */
	packet -> start_delimeter = 0x7E; // same for all packets
	packet -> length_lsb = (uint8_t)((uint8_t)0xFF & (5+size)); // +5 becuase of the other fields (frame type, frame id, target (16 bit), options)
	packet -> length_msb = (uint8_t)(((5+size) >> 8) & (uint8_t)0xFF);
	packet -> frame_type = 0x01; // trasmit, 16 bit address

	uint8_t array[sizeof(struct data_frame) + size + 1]; // create array to hold entire frame. +1 for the checksum

	uint8_t i = 0; // iterate through array and add the frame to it byte by byte
	array[i++] = packet -> start_delimeter;
	array[i++] = packet -> length_msb;
	array[i++] = packet -> length_lsb;
	array[i++] = packet -> frame_type;
	array[i++] = packet -> frame_id;
	array[i++] = packet -> dest_msb;
	array[i++] = packet -> dest_lsb;
	array[i++] = packet -> options;
	for (uint8_t c = 0; c < size; c++) {
		array[i++] = *(data_arr+c);
	}
	add_checksum(array, sizeof(array)); // array is made of uint8_t, so no division needed (sizeof(array)/sizeof(array[0]))

	send_message(array, sizeof(array)); // send message as defined in uart_xbee
}

/* For Command Frame (sets values on XBee itself) */
void cmd_add_frame_id(struct cmd_frame *packet, uint8_t id) {
	packet -> frame_id = id;
}

void cmd_send_frame(struct cmd_frame *packet, uint8_t cmd_1, uint8_t cmd_2, uint8_t *cmd_params, uint8_t size) {
	packet -> start_delimeter = 0x7E; // same for all frames
	packet -> frame_type = 0x08; // command frame
	packet -> at_cmd_1 = cmd_1;
	packet -> at_cmd_2 = cmd_2;
	packet -> length_lsb = (uint8_t)((uint8_t)0xFF & (4+size)); // +4 becuase extra fields are frame type, frame id, AT Command
	packet -> length_msb = (uint8_t)(((4+size) >> 8) & (uint8_t)0xFF);

	uint8_t array[sizeof(struct cmd_frame) + size + 1];

	uint8_t i = 0;
	array[i++] = packet -> start_delimeter;
	array[i++] = packet -> length_msb;
	array[i++] = packet -> length_lsb;
	array[i++] = packet -> frame_type;
	array[i++] = packet -> frame_id;
	array[i++] = packet -> at_cmd_1;
	array[i++] = packet -> at_cmd_2;
	if (cmd_params) {
		for (uint8_t c = 0; c < size; c++) {
			array[i++] = *(cmd_params+c);
		}
	}
	add_checksum(array, sizeof(array)); // array is made of uint8_t, so no division needed (sizeof(array)/sizeof(array[0]))

	send_message(array, sizeof(array));
}
