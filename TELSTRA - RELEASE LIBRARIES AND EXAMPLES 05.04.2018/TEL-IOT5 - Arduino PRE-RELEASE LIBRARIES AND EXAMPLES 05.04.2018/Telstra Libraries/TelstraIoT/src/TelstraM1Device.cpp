#include "TelstraM1Device.h"

/*
 * Initialise IoT Device
 */
TelstraM1Device::TelstraM1Device(TelstraM1Interface *commsif)
{
	this->commsif = commsif;
}

TelstraM1Device::~TelstraM1Device()
{

}

int TelstraM1Device::getStatus(char* resultBuffer, int length)
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	int result = TELSTRAIOT_STATUS_OK;

	packet_create_cmd(CMD_GET_STATUS, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);
	
	commsif->sendCommand(commandBuffer, c_size);

	result = commsif->getResponse(resultBuffer,length,M1DEVICE_CMD_TIMEOUT);

	//result < 0 means an error code
	if (result < 0) {
		reportln(REPORT_L3, "<M1Device:getStatus> - Response Timeout");
		return TELSTRAIOT_STATUS_ERROR;	
	}

	//Checking correct packet header
	if (packet_get_command(resultBuffer) != CMD_RES_OFFSET + CMD_GET_STATUS) {
		reportln(REPORT_L3, "<M1Device:getStatus> - Unknown response, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_ERROR;	
	}
	
	return TELSTRAIOT_STATUS_OK;
}

int TelstraM1Device::getIMEI(char* resultBuffer)
{
	char status[100];
	if (getStatus(status,100) == TELSTRAIOT_STATUS_OK) {	
		
		packet_get_param(resultBuffer, status, 3);

		return TELSTRAIOT_STATUS_OK;
	} else {
		reportln(REPORT_L3, "<M1Device:getIMEI> - Status timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_ERROR;
	}
}

int TelstraM1Device::getIP(char* resultBuffer)
{
	char status[100];
	if (getStatus(status,100) == TELSTRAIOT_STATUS_OK) {	
		
		packet_get_param(resultBuffer, status, 4);

		return TELSTRAIOT_STATUS_OK;
	} else {
		return TELSTRAIOT_STATUS_ERROR;
	}
}


int TelstraM1Device::getTime(char* resultBuffer)
{
	char status[100];
	if (getStatus(status,100) == TELSTRAIOT_STATUS_OK) {	
		
		packet_get_param(resultBuffer, status, 5);
		reportln(REPORT_L2, resultBuffer);

		return TELSTRAIOT_STATUS_OK;
	} else {
		return TELSTRAIOT_STATUS_ERROR;
	}
}

bool TelstraM1Device::isBG96Connected()
{
	char status[100];
	char tempBuffer[1];
	if (getStatus(status,100) == TELSTRAIOT_STATUS_OK) {	
		
		packet_get_param(tempBuffer, status, 0);
		if(tempBuffer[0]==1) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool TelstraM1Device::isSIMReady()
{
	char status[100];
	char tempBuffer[1];
	if (getStatus(status,100) == TELSTRAIOT_STATUS_OK) {	
		
		packet_get_param(tempBuffer, status, 1);
		if(tempBuffer[0]==1) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool TelstraM1Device::isPDPContextActive()
{
	char status[100];
	char tempBuffer[1];
	if (getStatus(status,100) == TELSTRAIOT_STATUS_OK) {	
		
		packet_get_param(tempBuffer, status, 2);
		if(tempBuffer[0]==1) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool TelstraM1Device::isCellularSystemReady()
{

	char status[100];
	if (getStatus(status,100) == TELSTRAIOT_STATUS_OK) {
		return true;
	} else {
		return false;
	}
}

int TelstraM1Device::waitUntilCellularSystemIsReady()
{
	return waitUntilCellularSystemIsReady(CELLULAR_READY_TIMEOUT);
}

int TelstraM1Device::waitUntilCellularSystemIsReady(int timeout)
{
	int attempts = 0;
	while(!isPDPContextActive()) {
		delay(1000);
		attempts++;

		if (attempts > (timeout*2)) {
			reportln(REPORT_L3, "<M1Device:waitUntilCellularSystemIsReady> - Response Timeout");
			return 	TELSTRAIOT_STATUS_TIMEOUT;
		} 
	}
	return TELSTRAIOT_STATUS_OK;
}

int TelstraM1Device::writeCredentials(const char *deviceID,const char *deviceTenant,const char *deviceUsername,const char *devicePassword)
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
	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE, M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_WRITE_CRED)
			return TELSTRAIOT_STATUS_OK;	
		else {
			reportln(REPORT_L3, "<M1Device:writeCredentials> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:writeCredentials> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;
	}
}


int TelstraM1Device::readCredentials(char *deviceID,char *deviceTenant,char *deviceUsername,char *devicePassword)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_READ_CRED, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_READ_CRED) {
			packet_get_param(deviceID,sharedBuffer,0);
			packet_get_param(deviceTenant,sharedBuffer,1);
			packet_get_param(deviceUsername,sharedBuffer,2);
			packet_get_param(devicePassword,sharedBuffer,3);
			return TELSTRAIOT_STATUS_OK;	
		} else {
			reportln(REPORT_L3, "<M1Device:readCredentials> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:readCredentials> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::clearCredentials()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_CLEAR_CRED, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_CLEAR_CRED){
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:clearCredentials> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:clearCredentials> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::resetModem()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_RESET_MODEM, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);
	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_RESET_MODEM){
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:resetModem> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:resetModem> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::getBatteryStatus()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_BATTERY_STATUS, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_GET_BATTERY_STATUS) {
			return (int)packet_get_numeric_param(sharedBuffer,0);
		} else {
			reportln(REPORT_L3, "<M1Device:getBatteryStatus> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:getBatteryStatus> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::getBatteryStateOfCharge()
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_BATTERY_SOC, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_GET_BATTERY_SOC) {
			return (int)packet_get_numeric_param(sharedBuffer,0);
		} else {
			reportln(REPORT_L3, "<M1Device:getBatteryStateOfCharge> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:getBatteryStateOfCharge> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::getTemperature(char *temperature)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_TEMPERATURE, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_GET_TEMPERATURE) {
			packet_get_param(temperature,sharedBuffer,0);
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:getTemperature> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:getTemperature> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::getLightLevel(char *lightLevel)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_LIGHT_LEVEL, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_GET_LIGHT_LEVEL) {
			packet_get_param(lightLevel,sharedBuffer,0);
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:getLightLevel> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:getLightLevel> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::getLightSensorState(char* resultBuffer, int len)
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_LIGHT_SENSOR_STATE, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);

	commsif->sendCommand(commandBuffer, c_size);

	if (commsif->getResponse(resultBuffer, len, M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(resultBuffer) == CMD_RES_OFFSET + CMD_GET_LIGHT_SENSOR_STATE) {
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:getLightSensorState> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:getLightSensorState> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

bool TelstraM1Device::isLightSensorDataValid() {
	char resultBuffer[100];
	if (getLightSensorState(resultBuffer, 100) == TELSTRAIOT_STATUS_OK)
		return packet_get_numeric_param(resultBuffer, 0) != 0;
	else return false;
}

bool TelstraM1Device::isLightSensorDataNew()
{
	char resultBuffer[100];
	if (getLightSensorState(resultBuffer, 100) == TELSTRAIOT_STATUS_OK)
		return packet_get_numeric_param(resultBuffer, 1) != 0;
	else return false;
}

int TelstraM1Device::getLightSensorParams(char* resultBuffer, int len)
{
	char commandBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_GET_LIGHT_SENSOR_PARAMS, commandBuffer, &c_checksum, &c_size);
	packet_append_ending(commandBuffer, &c_checksum, &c_size);

	commsif->sendCommand(commandBuffer, c_size);

	if (commsif->getResponse(resultBuffer, len, M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(resultBuffer) == CMD_RES_OFFSET + CMD_GET_LIGHT_SENSOR_PARAMS) {
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:getLightSensorParams> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:getLightSensorParams> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}
}

int TelstraM1Device::getLightSensorGain()
{
	char resultBuffer[100];

	if (getLightSensorParams(resultBuffer, 100) == TELSTRAIOT_STATUS_OK)
		return packet_get_numeric_param(resultBuffer, 0);
	else return TELSTRAIOT_STATUS_ERROR;
}

int TelstraM1Device::getLightSensorIntegrationTime()
{
	char resultBuffer[100];
	if (getLightSensorParams(resultBuffer, 100) == TELSTRAIOT_STATUS_OK)
		return packet_get_numeric_param(resultBuffer, 1);
	else return TELSTRAIOT_STATUS_ERROR;
}

int TelstraM1Device::getLightSensorMeasurementRate()
{
	char resultBuffer[100];
	if (getLightSensorParams(resultBuffer, 100) == TELSTRAIOT_STATUS_OK)
		return packet_get_numeric_param(resultBuffer, 2);
	else return TELSTRAIOT_STATUS_ERROR;	
}

int TelstraM1Device::setLightSensorParams(uint8_t gain, uint8_t int_rate, uint8_t meas_rate)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_SET_LIGHT_SENSOR_PARAMS, sharedBuffer, &c_checksum, &c_size);
	packet_append_bin_param((char *)&gain, 1, sharedBuffer, &c_checksum, &c_size);
	packet_append_bin_param((char *)&int_rate, 1, sharedBuffer, &c_checksum, &c_size);
	packet_append_bin_param((char *)&meas_rate, 1, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer, TX_BUFFER_SIZE, M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_SET_LIGHT_SENSOR_PARAMS) {
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:setLightSensorParams> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:setLightSensorParams> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}	
}

int TelstraM1Device::updateRTCFromNetwork(char* resultBuffer)
{
	char sharedBuffer[TX_BUFFER_SIZE];	
	char c_checksum = 0;
	uint16_t c_size = 0;

	packet_create_cmd(CMD_UPDATE_RTC, sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(sharedBuffer, &c_checksum, &c_size);

	commsif->sendCommand(sharedBuffer, c_size);

	if (commsif->getResponse(sharedBuffer,RX_BUFFER_SIZE,M1DEVICE_CMD_TIMEOUT) >= 0) {
		if (packet_get_command(sharedBuffer) == CMD_RES_OFFSET + CMD_UPDATE_RTC) {
			packet_get_param(resultBuffer,sharedBuffer,0);
			return TELSTRAIOT_STATUS_OK;
		} else {
			reportln(REPORT_L3, "<M1Device:updateRTCFromNetwork> - Unknown response, resetting...");
			commsif->resetCellularSystem();
			return TELSTRAIOT_STATUS_ERROR;
		}
	} else {
		reportln(REPORT_L3, "<M1Device:updateRTCFromNetwork> - Timeout, resetting...");
		commsif->resetCellularSystem();
		return TELSTRAIOT_STATUS_TIMEOUT;	
	}	
}

