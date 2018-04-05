#include "str_utils.h"

bool str_is_space(char data)
{
	if (data == ' ') return true;
	if (data == '\n') return true;
	if (data == '\r') return true;
	return false;
}

char * str_trim(char *text)
{
	char *start_pointer = text;
	char *end_pointer;
	
	while (str_is_space(*start_pointer)) start_pointer++;
	end_pointer = start_pointer + strlen(start_pointer) - 1;
	
	while (str_is_space(*end_pointer)) end_pointer--;
	
	*(end_pointer + 1) = '\0';

	return start_pointer;
}

bool str_ends_in(const char* pattern, char *text) {
	char *trimmed_text = str_trim(text);
	uint16_t length = strlen(trimmed_text);
	uint16_t length_pattern = strlen(pattern);
	
	if (length < length_pattern) return false;
	
	char * new_pointer = trimmed_text + length - length_pattern;
	
	if (strcmp(pattern, str_trim(new_pointer)) == 0) return true;
	return false;
}

bool mem_ends_in(const char* pattern, uint16_t p_size, char *buffer, uint16_t b_size) {
	if (b_size < p_size) return false;
	
	char * new_pointer = buffer + b_size - p_size;
	
	if (memcmp(pattern, new_pointer, p_size) == 0) return true;
	return false;	
}

void write_int8_to_char(char *output, uint8_t number)
{
	*output = (char)number;	
}

void write_int16_to_char(char *output, uint16_t number)
{
	*output = (char)(number >> 8);	//MSB
	*(output+1) = (char)number;		//LSB
}

void write_int32_to_char(char *output, uint32_t number)
{
	*output		= (char)(number >> 24);	//MSB
	*(output+1) = (char)(number >> 16);
	*(output+2) = (char)(number >> 8);
	*(output+3) = (char)number;			//LSB	
}

uint16_t read_int16_from_char(char *data)
{
	uint16_t result = *data;
	return (result << 8) + (uint8_t)*(data+1);
}

uint32_t read_int32_from_char(char *data)
{
	uint32_t result = *data;
	result = (result << 8) + *(data+1);
	result = (result << 8) + *(data+2);
	return (result << 8) + *(data+3);
}
