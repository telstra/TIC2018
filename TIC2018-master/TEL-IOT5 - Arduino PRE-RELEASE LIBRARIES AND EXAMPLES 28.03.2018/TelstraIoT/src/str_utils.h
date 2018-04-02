/*
 * str_utils.h
 *
 * Created: 1/02/2017 12:28:23 PM
 *  Author: Rodrigo Maureira
 */ 


#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

bool str_is_space(char data);
char * str_trim(char *text);
bool str_ends_in(const char* pattern, char *text);
bool mem_ends_in(const char* pattern, uint16_t p_size, char *buffer, uint16_t b_size);

void write_int8_to_char(char *output, uint8_t number);
void write_int16_to_char(char *output, uint16_t number);
void write_int32_to_char(char *output, uint32_t number);

uint16_t read_int16_from_char(char *data);
uint32_t read_int32_from_char(char *data);

#endif /* STR_UTILS_H_ */