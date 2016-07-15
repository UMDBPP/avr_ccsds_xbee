#include <stdint.h>
#include <string.h>

/* These are user facing macros to make adding/extracting data to/from a byte array easy */

// ADD_INT will take an int of any type and add it to the given array
#define ADD_INT(val, arr, arr_pos) ((arr_pos)+sizeof((val))); for(uint8_t ai = 0; ai < sizeof((val)); ai++) { *((arr)+ai+(arr_pos)-sizeof((val))) = (((val) >> (8*(sizeof((val))-(ai+1)))) & 0xFF); }

// ADD_FLOAT will take a floating point value and add it to the given array. Works for floats and doubles (same size on AVR)
// Unfortunately, this requires the union. Array positions are odd because of Endianess.
#define ADD_FLOAT(val, arr, arr_pos) ((arr_pos)+sizeof(float)); float_to_int.value=((float)(val)); for(uint8_t af = 0; af < sizeof(float); af++) { *((arr)+af+(arr_pos)-sizeof(float)) = float_to_int.bytes[sizeof(float)-1-af]; }

// EXTRACT_TLM will take the address of a variable, and then take the next set of bytes that correspond to the 
// variable type from the array and then stores it to the address. Unfortunately, this uses memcpy. The array positions
// are odd because of Endianess
#define EXTRACT_TLM(val, arr, arr_pos) ((arr_pos)+sizeof(*(val))); for (uint8_t xt = 0; xt < sizeof(*(val)); xt++) { tmp[sizeof(*(val))-(xt+1)] = *((arr)+(arr_pos)+xt); } memcpy((val), tmp, sizeof(*(val)));

static union {
	float value;
	uint8_t bytes[sizeof(float)];
} float_to_int;

static uint8_t tmp[8]; // temporary array for the EXTRACT_TLM piece. It should be fine to turn this to size 4
