#include "connection4g.h"

/*
 * Initialise connection to G55 and setup pins
 * secure_in defines security - if true, the G55 will attempt to use TLS
 */
Connection4G::Connection4G(bool secure, TelstraM1Interface *IoTDevice)
{
	this->secure = secure;
	this->IoTDevice = IoTDevice;
	this->socket_open = false;
	this->port = 0;
	this->hostname[0] = '\0';
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
	
	IoTDevice->sendCommand(commandBuffer, c_size);
	
	char pingResponse[100];
	
	if(IoTDevice->getResponse(pingResponse,100,CONNECTION4G_TCP_CONN_TIMEOUT)>0)
	{
		if (packet_get_command(pingResponse) == CMD_RES_OFFSET + CMD_PING) {
			packet_get_param(responseBuffer, pingResponse, 0);	
			int responseTime = packet_get_numeric_param(pingResponse, 1);	
			return responseTime;
		} else {
			reportln(REPORT_L2, "<Connection4G:ping> - Unknown response, resetting...");
			
			return CONNECTION4G_STATUS_ERROR;	
		}
	} else {
		reportln(REPORT_L2, "<Connection4G:ping> - Ping Timeout");
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
	
	IoTDevice->sendCommand(commandBuffer, c_size);
	
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
	
	IoTDevice->sendCommand(commandBuffer, c_size);
	
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
	
	if(IoTDevice->sendCommand(sharedBuffer, c_size)>=0){
		reportln(REPORT_L3, "<Connection4G:openTCP> - Sent command successfully...");
	
		if(IoTDevice->getResponse(sharedBuffer,RX_BUFFER_SIZE,CONNECTION4G_TCP_CONN_TIMEOUT)>0)
		{
			if(packet_get_command(sharedBuffer)==(cmd+CMD_RES_OFFSET))
			{
				this->socket_open = true;
				this->port = port;
				strcpy(this->hostname, host);
				return CONNECTION4G_STATUS_OK;
			} else {
				reportln(REPORT_L2, "<Connection4G:openTCP> - Unknown response, resetting...");
				
				return CONNECTION4G_STATUS_ERROR;
			}
		} else {
			reportln(REPORT_L2, "<Connection4G:openTCP> - Response Timeout");
			return CONNECTION4G_STATUS_ERROR;
		}
	}else{
		reportln(REPORT_L2, "<Connection4G:openTCP> - Error sending command to G55");
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
	uint8_t cmd;
	this->socket_open = false;

	if(secure)
	{
		cmd = CMD_CLOSE_SEC_TCP;
	} else {
		cmd = CMD_CLOSE_TCP;
	}
	packet_create_cmd(cmd, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	IoTDevice->sendCommand(commandBuffer, c_size);
	
	char responseBuffer[RX_BUFFER_SIZE];
	if(IoTDevice->getResponse(responseBuffer,RX_BUFFER_SIZE,CONNECTION4G_TCP_CONN_TIMEOUT) > 0)
	{
		if (packet_get_command(responseBuffer) == CMD_RES_OFFSET + cmd){
			return CONNECTION4G_STATUS_OK;	
		} else {
			reportln(REPORT_L2, "<Connection4G:closeTCP> - Unknown response, resetting...");
			
			return CONNECTION4G_STATUS_ERROR;			
		}		
	} else {
		reportln(REPORT_L2, "<Connection4G:closeTCP> - Timeout, resetting...");
		IoTDevice->resetCellularSystem();
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
	uint8_t cmd = 0;
	int res;

	if (this->socket_open) {
		if (openTCP(this->hostname, this->port) != CONNECTION4G_STATUS_OK) {
			reportln(REPORT_L2, "<Connection4G:TCPRead> - Can't establish connection, resetting...");
			
			return CONNECTION4G_STATUS_ERROR;				
		}		
	}

	if(secure) {
		cmd = CMD_TCP_SEC_READ;
	} else {
		cmd = CMD_TCP_READ;
	}

	packet_create_cmd(cmd, sharedBuffer, &c_checksum, &c_size);

	char lengthchar[2];
	write_int16_to_char(lengthchar, length);
	packet_append_bin_param(lengthchar, 2, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	
	IoTDevice->sendCommand(sharedBuffer, c_size);
	
	if(IoTDevice->getResponse(sharedBuffer,TX_BUFFER_SIZE,CONNECTION4G_TCP_DATA_TIMEOUT)>0)
	{
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + cmd){
			packet_get_param(buffer,sharedBuffer,0);
			int param_size = packet_get_param_size(sharedBuffer,0);
			
			if(param_size>0) {
				return param_size;
			} else {
				reportln(REPORT_L2, "<Connection4G:TCPRead> - Empty response");
				return CONNECTION4G_STATUS_ERROR;
			}
		} else {

			if ((packet_get_command(sharedBuffer) == CMD_SOCKET_NOT_READY) ||
				(packet_get_command(sharedBuffer) == CMD_CONNECTION_LOST) ) {
				reportln(REPORT_L2, "<Connection4G:TCPRead> - Connection lost, resetting...");				
			} else {
				reportln(REPORT_L2, "<Connection4G:TCPRead> - Unexpected response, resetting...");
			}
			this->socket_open = false;
			
			return CONNECTION4G_STATUS_ERROR;				
		}
	} else {
		reportln(REPORT_L2, "<Connection4G:TCPRead> - Command timeout");
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
	uint8_t cmd = 0;

	if (this->socket_open) {
		if (openTCP(this->hostname, this->port) != CONNECTION4G_STATUS_OK) {
			reportln(REPORT_L2, "<Connection4G:TCPWrite> - Can't establish connection, resetting...");
			
			return CONNECTION4G_STATUS_ERROR;				
		}		
	}	else{
		reportln(REPORT_L2, "<Connection4G:TCPWrite> - Socket Not Open...");
		return CONNECTION4G_STATUS_ERROR;
	}
	
	if(secure) {
		cmd = CMD_TCP_SEC_WRITE;
	} else {
		cmd = CMD_TCP_WRITE;
	}

	packet_create_cmd(cmd, commandBuffer, &c_checksum, &c_size);
	packet_append_param(data, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	IoTDevice->sendCommand(commandBuffer, c_size);

	char responseBuffer[RX_BUFFER_SIZE];
	int resp = IoTDevice->getResponse(responseBuffer,RX_BUFFER_SIZE,CONNECTION4G_TCP_CONN_TIMEOUT);
	
	if(resp>0) {
		if (packet_get_command(responseBuffer) == CMD_RES_OFFSET + cmd){
			return CONNECTION4G_STATUS_OK;
		} else {
			if ((packet_get_command(responseBuffer) == CMD_SOCKET_NOT_READY) ||
				(packet_get_command(responseBuffer) == CMD_CONNECTION_LOST) ) {
				reportln(REPORT_L2, "<Connection4G:TCPWrite> - Connection lost, resetting...");				
			} else {
				reportln(REPORT_L2, "<Connection4G:TCPWrite> - Unexpected response, resetting...");
			}

			IoTDevice->resetCellularSystem();
			return CONNECTION4G_STATUS_ERROR;				
		}
	} else {
		reportln(REPORT_L2, "<Connection4G:TCPWrite> - Command timeout, resetting...");
		
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
	
	IoTDevice->sendCommand(sharedBuffer, c_size);
	
	int resp = IoTDevice->getResponse(sharedBuffer,RX_BUFFER_SIZE,CONNECTION4G_TCP_CONN_TIMEOUT);
	
	if(resp>0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_GET_SIG_QTY) {
			packet_get_param(data,sharedBuffer,0);
			return CONNECTION4G_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:getSignalQuality> - Unknown response, resetting...");
			IoTDevice->resetCellularSystem();
			return CONNECTION4G_STATUS_ERROR;
		}		
	} else {
		reportln(REPORT_L3, "<M1Device:getSignalQuality> - Command timeout, resetting...");
		IoTDevice->resetCellularSystem();
		return CONNECTION4G_STATUS_ERROR;
	}
}
