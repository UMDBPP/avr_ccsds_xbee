#include <stdint.h>
#include <string.h>

/* These are user facing functions to make adding/extracting data to/from a byte array easy */
#define ADD_INT(val, arr, arr_pos) ((arr_pos)+sizeof((val))); for(uint8_t ai = 0; ai < sizeof((val)); ai++) { *((arr)+ai+(arr_pos)-sizeof((val))) = (((val) >> (8*(sizeof((val))-(ai+1)))) & 0xFF); }

#define ADD_FLOAT(val, arr, arr_pos) ((arr_pos)+sizeof(float)); float_to_int.value=((float)(val)); for(uint8_t af = 0; af < sizeof(float); af++) { *((arr)+af+(arr_pos)-sizeof(float)) = float_to_int.bytes[sizeof(float)-1-af]; }

#define EXTRACT_TLM(val, arr, arr_pos) ((arr_pos)+sizeof(*(val))); for (uint8_t xt = 0; xt < sizeof(*(val)); xt++) { tmp[sizeof(*(val))-(xt+1)] = *((arr)+(arr_pos)+xt); } memcpy((val), tmp, sizeof(*(val)));

static union {
	float value;
	uint8_t bytes[sizeof(float)];
} float_to_int;

static uint8_t tmp[8];
