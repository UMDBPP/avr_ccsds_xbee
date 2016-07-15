#include "ccsds_xbee.h"

/* unfortunate globals */
static struct data_frame data_packet;
static struct cmd_frame cmd_packet;
static uint8_t _packet_data[PKT_MAX_LEN];
static uint16_t send_ctr;
static uint16_t rx_ctr;
//static uint8_t cmd_rej_ctr;

/* packets called: data_packet, cmd_packet */
void init_XBee(uint16_t address, uint16_t pan_id) {
	send_ctr = 0;
	rx_ctr = 0;
	//_cmd_rej_ctr = 0;

	init_UART();

	cmd_add_frame_id(&cmd_packet, 0); // 0 means no cmd response

	// MY command to set xbee address
	uint8_t my_params[] = {(uint8_t)(address >> 8), (uint8_t)(address & (uint8_t)0xFF)};
	// ID command to set PAN ID
	uint8_t id_params[] = {(uint8_t)(pan_id >> 8), (uint8_t)(pan_id & (uint8_t)0xFF)};
	// AP command to set xbee to api mode
	uint8_t ap_params[] = {0x01};
	// CE command to set xbee to end node
	uint8_t ce_params[] = {0x00};
	// CH command to set xbee channel to c
	uint8_t ch_params[] = {0x0C};
	// BD command to set xbee baud rate to 9600
	uint8_t bd_params[] = {0x03};
	// PL command to set xbee power level to max
	uint8_t pl_params[] = {0x04};

	// send  MY command
	cmd_send_frame(&cmd_packet, (uint8_t)'M', (uint8_t)'Y', my_params, (uint8_t)sizeof(my_params));
	// AC command to apply changes
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'C', NULL, 0);
	// send ID command
	cmd_send_frame(&cmd_packet, (uint8_t)'I', (uint8_t)'D', id_params, (uint8_t)sizeof(id_params));
	// AC command to apply changes
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'C', NULL, 0);
	// send AP command
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'P', ap_params, (uint8_t)sizeof(ap_params));
	// AC command to apply changes
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'C', NULL, 0);
	// send CE command
	cmd_send_frame(&cmd_packet, (uint8_t)'C', (uint8_t)'E', ce_params, (uint8_t)sizeof(ce_params));
	// AC command to apply changes
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'C', NULL, 0);
	// send CH command
	cmd_send_frame(&cmd_packet, (uint8_t)'C', (uint8_t)'H', ch_params, (uint8_t)sizeof(ch_params));
	// AC command to apply changes
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'C', NULL, 0);
	// send BD command
	cmd_send_frame(&cmd_packet, (uint8_t)'B', (uint8_t)'D', bd_params, (uint8_t)sizeof(bd_params));
	// AC command to apply changes
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'C', NULL, 0);
	// send PL command
	cmd_send_frame(&cmd_packet, (uint8_t)'P', (uint8_t)'L', pl_params, (uint8_t)sizeof(pl_params));
	// AC command to apply changes
	cmd_send_frame(&cmd_packet, (uint8_t)'A', (uint8_t)'C', NULL, 0);
	// WR command to save current settings to non volatile memory
	cmd_send_frame(&cmd_packet, (uint8_t)'W', (uint8_t)'R', NULL, 0);
}

uint8_t send_tlm_msg(uint16_t send_addr, uint8_t *payload, uint8_t payload_size) {
	// TODO: Add multipacket support
	if (payload_size + sizeof(CCSDS_TlmPkt_t) > PKT_MAX_LEN) {
		// packet is too large
		return -1;
	}
	// create buffer large enough for data and other ccsds packet stuff
	uint8_t packet_data[payload_size + sizeof(CCSDS_TlmPkt_t)];

	CCSDS_PriHdr_t pri_header = *(CCSDS_PriHdr_t*) packet_data;
	CCSDS_TlmSecHdr_t tlm_sec_header = *(CCSDS_TlmSecHdr_t*) (packet_data+sizeof(CCSDS_PriHdr_t));

	// fill primary header fields
	CCSDS_WR_APID(pri_header,send_addr);
	CCSDS_WR_SHDR(pri_header,1);
	CCSDS_WR_TYPE(pri_header,0);
	CCSDS_WR_VERS(pri_header,0);
	CCSDS_WR_SEQ(pri_header, send_ctr);
	CCSDS_WR_SEQFLG(pri_header,0x03);
	CCSDS_WR_LEN(pri_header,payload_size+sizeof(CCSDS_TlmPkt_t));

	// fill secondary header fields
	// TODO: Write the functionality to keep time
	CCSDS_WR_SEC_HDR_SEC(tlm_sec_header, (uint32_t)0);
	CCSDS_WR_SEC_HDR_SUBSEC(tlm_sec_header, (uint32_t)0);

	// copy packet data
	memcpy(packet_data + sizeof(CCSDS_TlmPkt_t), payload, payload_size);

	// calculate final packet size
	payload_size += sizeof(CCSDS_TlmPkt_t);

	// send message
	data_add_target(&data_packet, send_addr);
	data_add_frame_id(&data_packet, (uint8_t)((send_ctr % 256) + 1)); // 0 for no ack
	data_add_options(&data_packet, (uint8_t)0);
	data_send_frame(&data_packet, packet_data, payload_size);
	send_ctr++;

	//return success
	return 1; // This should be return 0, but consistency with the Arduino setup
}

uint8_t send_cmd_msg(uint16_t send_addr, uint8_t fcn_code, uint8_t *payload, uint8_t payload_size) {
	if (payload_size + sizeof(CCSDS_CmdPkt_t) > PKT_MAX_LEN) {
		// packet is too long
		return -1;
	}

	// create buffer large enough for data and packet
	uint8_t packet_data[payload_size + sizeof(CCSDS_CmdPkt_t)];

	// cast the buffer into the header structures so that the fields can be filled
	CCSDS_PriHdr_t pri_header = *(CCSDS_PriHdr_t*) packet_data;
	CCSDS_CmdSecHdr_t cmd_sec_header = *(CCSDS_CmdSecHdr_t*) (packet_data+sizeof(CCSDS_PriHdr_t));

	// fill primary header fields
	CCSDS_WR_APID(pri_header,send_addr);
	CCSDS_WR_SHDR(pri_header,1);
	CCSDS_WR_TYPE(pri_header,1);
	CCSDS_WR_VERS(pri_header,0);
	CCSDS_WR_SEQ(pri_header, send_ctr);
	CCSDS_WR_SEQFLG(pri_header,0x03); 
	CCSDS_WR_LEN(pri_header,payload_size+sizeof(CCSDS_CmdPkt_t));

	// fill secondary header fields
	CCSDS_WR_CHECKSUM(cmd_sec_header, 0x0);
	CCSDS_WR_FC(cmd_sec_header, fcn_code);

	// write the checksum after the header's been added
	CCSDS_WR_CHECKSUM((*(CCSDS_CmdPkt_t*) packet_data).SecHdr, CCSDS_ComputeCheckSum((CCSDS_CmdPkt_t*) packet_data));

	// copy the packet data
	memcpy(packet_data+sizeof(CCSDS_CmdPkt_t), payload, payload_size);

	// update the payload_size to include the headers
	payload_size += sizeof(CCSDS_CmdPkt_t);

	// send message
	data_add_target(&data_packet, send_addr);
	data_add_frame_id(&data_packet, (uint8_t)((send_ctr % 256) + 1)); // 0 for no ack
	data_add_options(&data_packet, (uint8_t)0);
	data_send_frame(&data_packet, packet_data, payload_size);
	send_ctr++;

	// return success
	return 1; // eventually change to 0
}

uint8_t read_msg(uint16_t timeout) {
	//while not timeout
	uint8_t status = read_rx();
	if ((status == 0x81) || (status == 0x01)) {
		//uint8_t received_data[get_payload_length()];
		//cpy_data(received_data);
		cpy_data(_packet_data);
		CCSDS_PriHdr_t pri_header;
		pri_header = *(CCSDS_PriHdr_t*) (_packet_data);
		rx_ctr++;
		return (uint8_t)CCSDS_RD_TYPE(pri_header);
	}
	// end while
	return -1; // did not read anything
}

uint8_t read_tlm_msg(uint8_t *data) {
	CCSDS_PriHdr_t pri_header;
	pri_header = *(CCSDS_PriHdr_t*)(_packet_data);
	if (!CCSDS_RD_SHDR(pri_header)) {
		CCSDS_TlmSecHdr_t tlm_sec_header;
		tlm_sec_header = *(CCSDS_TlmSecHdr_t*) (_packet_data + sizeof(CCSDS_PriHdr_t));

		// copy data into user's array
		memcpy(data, _packet_data + sizeof(CCSDS_TlmPkt_t), (get_payload_length() - sizeof(CCSDS_TlmPkt_t)));
		// return data length
		return (CCSDS_RD_LEN(pri_header) - sizeof(CCSDS_TlmPkt_t));
	}

	return -1; // error
}

uint8_t read_cmd_msg(uint8_t *params, uint8_t *fcn_code) {
	CCSDS_PriHdr_t pri_header;
	pri_header = *(CCSDS_PriHdr_t*)(_packet_data);

	if(CCSDS_RD_SHDR(pri_header)) {
		CCSDS_CmdSecHdr_t cmd_sec_header;
		cmd_sec_header = *(CCSDS_CmdSecHdr_t*) (_packet_data+sizeof(CCSDS_PriHdr_t));
		// copy parameters into user's array
		memcpy(params, _packet_data + sizeof(CCSDS_CmdPkt_t), (get_payload_length() - sizeof(CCSDS_CmdPkt_t)));
		// copy fcn_code to pointer
		*fcn_code = CCSDS_RD_FC(cmd_sec_header);
		return (get_payload_length() - sizeof(CCSDS_CmdPkt_t)); // return size params
	}

	return -1; // error
}

uint16_t sent_messages() {
	return send_ctr;
}

uint16_t received_messages() {
	return rx_ctr;
}
