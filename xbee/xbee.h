#ifndef __xbee__
#define __xbee__

#include "../uart_xbee/uart_xbee.h"
#include <stddef.h>

/* definitional constants */
#define MAX_SIZE 127

/* structure definition */
struct data_frame {
	uint8_t start_delimeter;
	uint8_t length_msb;
	uint8_t length_lsb;
	uint8_t frame_type;
	uint8_t frame_id;
	uint8_t dest_msb;
	uint8_t dest_lsb;
	uint8_t options;
//	uint8_t data[CCSDS_MAX_SIZE];
//	uint8_t checksum;
};

struct cmd_frame {
	uint8_t start_delimeter;
	uint8_t length_msb;
	uint8_t length_lsb;
	uint8_t frame_type;
	uint8_t frame_id;
	uint8_t at_cmd_1;
	uint8_t at_cmd_2;
	// uint8_t param_vals[CCSDS_MAX_SIZE];
	// uint8_t checksum;
};

/*struct modem_status {
	uint8_t start_delimeter;
	uint8_t length_msb;
	uint8_t length_lsb;
	uint8_t frame_type;
	uint8_t status;
	// uint8_t checksum;
};*/

/* data function prototypes write */
void data_add_target(struct data_frame *packet, uint16_t target);
void data_add_frame_id(struct data_frame *packet, uint8_t id);
void data_add_options(struct data_frame *packet, uint8_t option_choice);
void data_send_frame(struct data_frame *packet, uint8_t *data_arr, uint8_t size);

/* command functions prototypes */
void cmd_add_frame_id(struct cmd_frame *packet, uint8_t id);
void cmd_send_frame(struct cmd_frame *packet, uint8_t cmd_1, uint8_t cmd_2, uint8_t *cmd_params, uint8_t size);

#endif // __xbee__
