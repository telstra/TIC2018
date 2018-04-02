#include "TelstraM1Device.h"

/*
 * Initialise IoT Device
 */
TelstraM1Device::TelstraM1Device(TelstraM1Interface *shield)
{
	this->shield = shield;
}

TelstraM1Device::~TelstraM1Device()
{

}

int TelstraM1Device::getStatus(char* resultBuffer, int length)
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_STATUS, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(commandBuffer, c_size);
	
	return shield->getResponse(resultBuffer,length,5000);
}

void TelstraM1Device::getIMEI(char* resultBuffer)
{
	char status[100];
	getStatus(status,100);

	//char imei[15];
	packet_get_param(resultBuffer, status, 3);
}

void TelstraM1Device::getIP(char* resultBuffer)
{
	char status[100];
	getStatus(status,100);

	//char imei[15];
	packet_get_param(resultBuffer, status, 4);	
}


void TelstraM1Device::getTime(char* resultBuffer)
{
	char status[100];
	getStatus(status,100);

	//char imei[15];
	packet_get_param(resultBuffer, status, 5);	
}
bool TelstraM1Device::isBG96Connected()
{
	char status[100];
	getStatus(status,100);

	char tempBuffer[1];
	packet_get_param(tempBuffer, status, 0);
	if(tempBuffer[0]==1)
	{
		return true;
	} else {
		return false;
	}
}
bool TelstraM1Device::isSIMReady()
{
	char status[100];
	getStatus(status,100);

	char tempBuffer[1];
	packet_get_param(tempBuffer, status, 1);
	if(tempBuffer[0]==1)
	{
		return true;
	} else {
		return false;
	}
}
bool TelstraM1Device::isPDPContextActive()
{
	char status[100];
	getStatus(status,100);

	char tempBuffer[1];
	packet_get_param(tempBuffer, status, 2);
	if(tempBuffer[0]==1)
	{
		return true;
	} else {
		return false;
	}
}

bool TelstraM1Device::isCellularSystemReady()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_STATUS, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	
	shield->sendCommand(sharedBuffer, c_size);
	
	shield->getResponse(sharedBuffer,TX_BUFFER_SIZE,500);

	if(packet_get_command(sharedBuffer)==100)
	{
		return true;
	} else {
		return false;
	}
}

void TelstraM1Device::waitUntilCellularSystemIsReady()
{
	while(!isCellularSystemReady())
	{
		delay(500);
	}
	while(!isPDPContextActive())
	{
		delay(500);
	}

}

void TelstraM1Device::writeCredentials(const char *deviceID,const char *deviceTenant,const char *deviceUsername,const char *devicePassword)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_WRITE_CRED, sharedBuffer, &c_checksum, &c_size);
	packet_append_param(deviceID, sharedBuffer, &c_checksum, &c_size);
	packet_append_param(deviceTenant, sharedBuffer, &c_checksum, &c_size);
	packet_append_param(deviceUsername, sharedBuffer, &c_checksum, &c_size);
	packet_append_param(devicePassword, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	shield->sendCommand(sharedBuffer, c_size);

	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);

}


void TelstraM1Device::readCredentials(char *deviceID,char *deviceTenant,char *deviceUsername,char *devicePassword)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_READ_CRED, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);
	packet_get_param(deviceID,sharedBuffer,0);
	packet_get_param(deviceTenant,sharedBuffer,1);
	packet_get_param(deviceUsername,sharedBuffer,2);
	packet_get_param(devicePassword,sharedBuffer,3);
}

void TelstraM1Device::clearCredentials()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_CLEAR_CRED, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);

}

void TelstraM1Device::resetModem()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_RESET_MODEM, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);

}

int TelstraM1Device::getBatteryStatus()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_BATTERY_STATUS, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);
	int resp = (int)packet_get_numeric_param(sharedBuffer,0);
	return resp;
}

int TelstraM1Device::getBatteryStateOfCharge()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_BATTERY_SOC, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);
	int resp = (int)packet_get_numeric_param(sharedBuffer,0);
	return resp;
}

void TelstraM1Device::getTemperature(char *temperature)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_TEMPERATURE, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);
	packet_get_param(temperature,sharedBuffer,0);

}

void TelstraM1Device::getLightLevel(char *lightLevel)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_LIGHT_LEVEL, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);
	packet_get_param(lightLevel,sharedBuffer,0);

}

void TelstraM1Device::getLightSensorState(char* resultBuffer, int len)
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_LIGHT_SENSOR_STATE, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);

	shield->sendCommand(commandBuffer, c_size);
	shield->getResponse(resultBuffer, len, 5000);
}

bool TelstraM1Device::isLightSensorDataValid()
{
	char resultBuffer[100];
	getLightSensorState(resultBuffer, 100);
	return packet_get_numeric_param(resultBuffer, 0) != 0;
}

bool TelstraM1Device::isLightSensorDataNew()
{
	char resultBuffer[100];
	getLightSensorState(resultBuffer, 100);
	return packet_get_numeric_param(resultBuffer, 1) != 0;
}

void TelstraM1Device::getLightSensorParams(char* resultBuffer, int len)
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_LIGHT_SENSOR_PARAMS, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);

	shield->sendCommand(commandBuffer, c_size);
	shield->getResponse(resultBuffer, len, 5000);
}

uint8_t TelstraM1Device::getLightSensorGain()
{
	char resultBuffer[100];
	getLightSensorParams(resultBuffer, 100);
	return (uint8_t)packet_get_numeric_param(resultBuffer, 0);
}

uint8_t TelstraM1Device::getLightSensorIntegrationTime()
{
	char resultBuffer[100];
	getLightSensorParams(resultBuffer, 100);
	return (uint8_t)packet_get_numeric_param(resultBuffer, 1);
}

uint8_t TelstraM1Device::getLightSensorMeasurementRate()
{
	char resultBuffer[100];
	getLightSensorParams(resultBuffer, 100);
	return (uint8_t)packet_get_numeric_param(resultBuffer, 2);
}

void TelstraM1Device::setLightSensorParams(uint8_t gain, uint8_t int_rate, uint8_t meas_rate)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_SET_LIGHT_SENSOR_PARAMS, sharedBuffer, &c_checksum, &c_size);
	packet_append_bin_param((char *)&gain, 1, sharedBuffer, &c_checksum, &c_size);
	packet_append_bin_param((char *)&int_rate, 1, sharedBuffer, &c_checksum, &c_size);
	packet_append_bin_param((char *)&meas_rate, 1, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer, TX_BUFFER_SIZE, 5000);
}

void TelstraM1Device::updateRTCFromNetwork(char* resultBuffer)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_UPDATE_RTC, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	shield->sendCommand(sharedBuffer, c_size);
	shield->getResponse(sharedBuffer,RX_BUFFER_SIZE,5000);
	packet_get_param(resultBuffer,sharedBuffer,0);

}

