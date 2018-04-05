#include <string.h>
#include <stdint.h>
#include "str_utils.h"
#include "Arduino.h"

/* Function Prototypes: Packet creation */
void packet_create_cmd(uint8_t command, char *output_buffer, char *current_checksum, uint16_t *current_size);
void packet_append_param(const char *param, char *output_buffer, char *current_checksum, uint16_t *current_size);
void packet_append_bin_param(const char *param, uint16_t size, char *output_buffer, char *current_checksum, uint16_t *current_size);
void packet_append_ending(char *output_buffer, char *current_checksum, uint16_t *current_size);

/* Function Prototypes: Packet reading */
uint8_t packet_compute_checksum(char *packet, uint16_t size);
bool packet_validate_checksum(char *packet, uint16_t size);
uint8_t packet_get_command(char *packet);
void packet_get_param(char *dest, char *packet, uint8_t number);
uint16_t packet_get_param_size(char *packet, uint8_t number);
uint32_t packet_get_numeric_param(char *packet, uint8_t number);

/* General utilities */
unsigned int getLength(char a[]); 