#include "packet_utils.h"

/* Command packet formatting and decoding */
void packet_create_cmd(uint8_t command, char *output_buffer, char *current_checksum, uint16_t *current_size)
{
	/* We are assuming memory already allocated for TX Buffer, we manipulate it using pointers	*/
	*output_buffer = (char)command;
	*current_checksum = (char)command;
	*current_size = 1;
}

void packet_append_param(const char *param, char *output_buffer, char *current_checksum, uint16_t *current_size)
{
	packet_append_bin_param(param, strlen(param), output_buffer, current_checksum, current_size);
}

void packet_append_bin_param(const char *param, uint16_t size, char *output_buffer, char *current_checksum, uint16_t *current_size)
{

	/* More manipulation of the array using pointers and updating checksum and current size */
	uint16_t p_index;
	uint16_t p_length = size;
	char *tx_ptr;
	const char *param_ptr;
	
	tx_ptr = output_buffer + *current_size;
	param_ptr = param;
	
	*tx_ptr = (char)(p_length>>8);	//MSB Parameter length
	*current_checksum += *tx_ptr;
	tx_ptr++;
	
	*tx_ptr = (char)(p_length);		//LSB Parameter length
	*current_checksum += *tx_ptr;
	tx_ptr++;
	
	//This copies bytes from the input buffer to the output buffer and calculates the checksum
	for(p_index = 0; p_index < p_length; p_index++){
		*tx_ptr = *(param_ptr+p_index);
		*current_checksum += *tx_ptr;
		tx_ptr++;
		//param_ptr++;
	}
	
	*current_size += p_length + 2;	
}

void packet_append_ending(char *output_buffer, char *current_checksum, uint16_t *current_size)
{
	*(output_buffer + (*current_size)) = *current_checksum;
	(*current_size)++;
}

uint8_t packet_compute_checksum(char *packet, uint16_t size)
{
	uint16_t index = 0;
	uint8_t computed_checksum = 0;
	while (index < size - 1) {
		computed_checksum += (uint8_t)(*(packet + index));
		index++;
	}
	
	return computed_checksum;	
}

bool packet_validate_checksum(char *packet, uint16_t size)
{	
	uint8_t computed_checksum = packet_compute_checksum(packet, size);
	uint8_t received_checksum = ((uint8_t)(*(packet + size - 1)));
	
	/* Comparing the last byte with the computed checksum */
	if (computed_checksum == received_checksum)
	return true;
	
	return false;
}

uint8_t packet_get_command(char *packet)
{
	return (uint8_t)(*packet);	//1st byte is the command
}

void packet_get_param(char *dest, char *packet, uint8_t number)
{
	uint8_t current_param = 0;
	uint16_t param_size;
	char *param_ptr;
	param_ptr = packet + 1;
	
	for (current_param = 0; current_param < number; current_param++) {
		param_size = (uint16_t)(*param_ptr) << 8;	//MSB of size
		param_ptr++;
		param_size += ((uint16_t)(*param_ptr))&0xFF;		//LSB of size
		param_ptr++;
		param_ptr += param_size;
	}
	
	/* This assumes there is at least 1 parameter and 'number' holds a valid number */
	param_size = (uint16_t)(*param_ptr) << 8;	//MSB of size
	param_ptr++;
	param_size += ((uint16_t)(*param_ptr))&0xFF;		//LSB of size
	param_ptr++;
	
	/* Now we are ready to copy */
	memcpy(dest, param_ptr, param_size);
	*(dest + param_size) = '\0';	//Don't forget the termination character

}

uint16_t packet_get_param_size(char *packet, uint8_t number)
{
	uint8_t current_param = 0;
	uint16_t param_size;
	char *param_ptr;
	param_ptr = packet + 1;
	
	for (current_param = 0; current_param < number; current_param++) {
		param_size = (uint16_t)(*param_ptr) << 8;	//MSB of size
		param_ptr++;
		param_size += ((uint16_t)(*param_ptr))&0xFF;		//LSB of size
		param_ptr++;
		param_ptr += param_size;
	}
	
	/* This assumes there is at least 1 parameter and 'number' holds a valid number */
	param_size = (uint16_t)(*param_ptr) << 8;	//MSB of size
	param_ptr++;
	param_size += ((uint16_t)(*param_ptr))&0xFF;		//LSB of size
	param_ptr++;

	return param_size;
}

uint32_t packet_get_numeric_param(char *packet, uint8_t number)
{
	uint16_t param_size = packet_get_param_size(packet, number);

	if (param_size > 4) return 0;
	char param[5];
	packet_get_param(param, packet, number);

	if (param_size == 1) return (uint32_t)(*param);
	if (param_size == 2) return (uint32_t)read_int16_from_char(param);
	if (param_size == 4) return read_int32_from_char(param);
	
	return 0;
}

unsigned int getLength(char a[])
{
	// Get length of char array 'a'
	return sizeof(a)/sizeof(*a);
}
