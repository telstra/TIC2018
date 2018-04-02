#include "connection4g.h"

// TODO - handle error response codes (codes 200+) from shield


/*
 * Initialise connection to G55 and setup pins
 * secure_in defines security - if true, the G55 will attempt to use TLS
 */
Connection4G::Connection4G(bool secure, TelstraM1Interface *shield)
{
	this->secure = secure;
	this->shield = shield;
}


/*
 * Close connection to G55
 */
Connection4G::~Connection4G()
{

}

/*
 * Send ICMP ping message to specified host and store the result in a buffer
 */
int Connection4G::ping(const char host[], char responseBuffer[])
{
	// Send a ping command to the G55
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_PING, commandBuffer, &c_checksum, &c_size);
	packet_append_param(host, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(commandBuffer, c_size);
	
	char pingResponse[100];
	
	if(shield->getResponse(pingResponse,100,5000)>0)
	{	
		packet_get_param(responseBuffer, pingResponse, 0);	
		int responseTime = packet_get_numeric_param(pingResponse, 1);	
		return responseTime;
	} else {
		return CONNECTION4G_STATUS_ERROR;
	}
}

/*
 * Activate PDP context on the 4G module
 */
int Connection4G::activatePDP(const char apn[], const char username[], const char password[])
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_ACTIVATE_PDP, commandBuffer, &c_checksum, &c_size);
	packet_append_param(apn, commandBuffer, &c_checksum, &c_size);
	packet_append_param(username, commandBuffer, &c_checksum, &c_size);
	packet_append_param(password, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(commandBuffer, c_size);
	
	return CONNECTION4G_STATUS_OK;
}

/*
 * Deactivate PDP context
 */
int Connection4G::deactivatePDP()
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_DEACTIVATE_PDP, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(commandBuffer, c_size);
	
	return CONNECTION4G_STATUS_OK;
}

/*
 * Open TCP connection using the specified host name and port
 * Uses TLS if secure=true was set during Connection4G initialisation
 */
int Connection4G::openTCP(const char host[], uint16_t port)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;



	uint8_t cmd;
	if(secure)
	{
		cmd = CMD_OPEN_SEC_TCP;
	} else {
		cmd = CMD_OPEN_TCP;
	}
	packet_create_cmd(cmd, sharedBuffer, &c_checksum, &c_size);

	packet_append_param(host, sharedBuffer, &c_checksum, &c_size);
	
	char portchar[2];
	write_int16_to_char(portchar, port);
	
	packet_append_bin_param(portchar, 2, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(sharedBuffer, c_size);
	
	if(shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,30000)>0)
	{
		if(packet_get_command(sharedBuffer)==(cmd+CMD_RES_OFFSET))
		{
			return CONNECTION4G_STATUS_OK;
		} else {
			return CONNECTION4G_STATUS_ERROR;
		}
	} else {
		return CONNECTION4G_STATUS_ERROR;
	}

}

/*
 * Closes an open TCP connection
 */
int Connection4G::closeTCP()
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	if(secure)
	{
		packet_create_cmd(CMD_CLOSE_SEC_TCP, commandBuffer, &c_checksum, &c_size);
	} else {
		packet_create_cmd(CMD_CLOSE_TCP, commandBuffer, &c_checksum, &c_size);
	}
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(commandBuffer, c_size);
	
	char responseBuffer[RX_BUFFER_SIZE];
	if(shield->getResponse(responseBuffer,RX_BUFFER_SIZE,15000)>0)
	{
		return CONNECTION4G_STATUS_OK;
	} else {
		return CONNECTION4G_STATUS_ERROR;
	}
}

/*
 * Reads data from a TCP connection
 */
int Connection4G::TCPRead(char buffer[], uint16_t length)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	if(secure)
	{
		packet_create_cmd(CMD_TCP_SEC_READ, sharedBuffer, &c_checksum, &c_size);
	} else {
		packet_create_cmd(CMD_TCP_READ, sharedBuffer, &c_checksum, &c_size);
	}
	char lengthchar[2];
	write_int16_to_char(lengthchar, length);
	packet_append_bin_param(lengthchar, 2, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(sharedBuffer, c_size);
	
	if(shield->getResponse(sharedBuffer,TX_BUFFER_SIZE,5000)>0)
	{
		packet_get_param(buffer,sharedBuffer,0);

		int param_size = packet_get_param_size(sharedBuffer,0);
		
		if(param_size>0) {
			return param_size;
		} else {
			return CONNECTION4G_STATUS_ERROR;
		}
	} else {
		return CONNECTION4G_STATUS_ERROR;
	}
}

/*
 * Sends TCP packet to host
 */
int Connection4G::TCPWrite(char* data, uint16_t length)
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;
	
	if(secure)
	{
		packet_create_cmd(CMD_TCP_SEC_WRITE, commandBuffer, &c_checksum, &c_size);
	} else {
		packet_create_cmd(CMD_TCP_WRITE, commandBuffer, &c_checksum, &c_size);
	}
	packet_append_param(data, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(commandBuffer, c_size);
	

	char responseBuffer[RX_BUFFER_SIZE];
	int resp = shield->getResponse(responseBuffer,RX_BUFFER_SIZE,5000);
	
	if(resp>0)
	{
		return CONNECTION4G_STATUS_OK;
	} else {
		return CONNECTION4G_STATUS_ERROR;
	}


}

int Connection4G::getSignalQuality(char* data)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;
	

	packet_create_cmd(CMD_GET_SIG_QTY, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(sharedBuffer, c_size);
	
	int resp = shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);
	
	packet_get_param(data,sharedBuffer,0);

	if(resp>0)
	{
		return CONNECTION4G_STATUS_OK;
	} else {
		return CONNECTION4G_STATUS_ERROR;
	}


}
