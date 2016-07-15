#include "xbee.h"

/* private helper method */
static void add_checksum(uint8_t *array, uint8_t size) {
	uint16_t value = 0;
	for (uint8_t i = 3; i < (size-1); i++) {
		value += *(array+i);
	}
	value = ((uint8_t)0xFF & value);

	*(array+(size-1)) = (0xFF - (uint8_t)value);
}

/* For data_frame type */
void data_add_target(struct data_frame *packet, uint16_t target) {
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
	packet -> start_delimeter = 0x7E;
	packet -> length_lsb = (uint8_t)((uint8_t)0xFF & (5+size));
	packet -> length_msb = (uint8_t)(((5+size) >> 8) & (uint8_t)0xFF);
	packet -> frame_type = 0x01; // trasmit, 16 bit address

	uint8_t array[sizeof(struct data_frame) + size + 1];

	uint8_t i = 0;
	array[i] = packet -> start_delimeter;
	i++;
	array[i] = packet -> length_msb;
	i++;
	array[i] = packet -> length_lsb;
	i++;
	array[i] = packet -> frame_type;
	i++;
	array[i] = packet -> frame_id;
	i++;
	array[i] = packet -> dest_msb;
	i++;
	array[i] = packet -> dest_lsb;
	i++;
	array[i] = packet -> options;
	i++;
	for (uint8_t c = 0; c < size; c++) {
		array[i] = *(data_arr+c);
		i++;
	}
	add_checksum(array, sizeof(array)); // array is made of uint8_t, so no division needed (sizeof(array)/sizeof(array[0]))

	send_message(array, sizeof(array));
}

/* for command frame */
void cmd_add_frame_id(struct cmd_frame *packet, uint8_t id) {
	packet -> frame_id = id;
}

void cmd_send_frame(struct cmd_frame *packet, uint8_t cmd_1, uint8_t cmd_2, uint8_t *cmd_params, uint8_t size) {
	packet -> start_delimeter = 0x7E;
	packet -> frame_type = 0x08;
	packet -> at_cmd_1 = cmd_1;
	packet -> at_cmd_2 = cmd_2;
	packet -> length_lsb = (uint8_t)((uint8_t)0xFF & (4+size));
	packet -> length_msb = (uint8_t)(((4+size) >> 8) & (uint8_t)0xFF);

	uint8_t array[sizeof(struct cmd_frame) + size + 1];

	uint8_t i = 0;
	array[i] = packet -> start_delimeter;
	i++;
	array[i] = packet -> length_msb;
	i++;
	array[i] = packet -> length_lsb;
	i++;
	array[i] = packet -> frame_type;
	i++;
	array[i] = packet -> frame_id;
	i++;
	array[i] = packet -> at_cmd_1;
	i++;
	array[i] = packet -> at_cmd_2;
	i++;
	if (cmd_params) {
		for (uint8_t c = 0; c < size; c++) {
			array[i] = *(cmd_params+c);
			i++;
		}
	}
	add_checksum(array, sizeof(array)); // array is made of uint8_t, so no division needed (sizeof(array)/sizeof(array[0]))

	send_message(array, sizeof(array));
}

/* for all packet types */
/*uint16_t read_length(uint8_t lsb, uint8_t msb) {
	return (uint16_t)((msb << 8) | (lsb & (uint8_t)0xFF));
}*/
