#ifndef TelstraM1Device_H
#define TelstraM1Device_H

#include <stdint.h>
#include <string.h>
#include "Arduino.h"
#include "shieldcommands.h"
#include "TelstraM1Interface.h"
#include "packet_utils.h"

#define LIGHT_SENSOR_GAIN_1X 0
#define LIGHT_SENSOR_GAIN_2X 1
#define LIGHT_SENSOR_GAIN_4X 2
#define LIGHT_SENSOR_GAIN_8X 3
#define LIGHT_SENSOR_GAIN_48X 6
#define LIGHT_SENSOR_GAIN_96X 7

#define LIGHT_SENSOR_INT_TIME_50MS 1
#define LIGHT_SENSOR_INT_TIME_100MS 0
#define LIGHT_SENSOR_INT_TIME_150MS 4
#define LIGHT_SENSOR_INT_TIME_200MS 2
#define LIGHT_SENSOR_INT_TIME_250MS 5
#define LIGHT_SENSOR_INT_TIME_300MS 6
#define LIGHT_SENSOR_INT_TIME_350MS 7
#define LIGHT_SENSOR_INT_TIME_400MS 3

#define LIGHT_SENSOR_MEAS_RATE_50MS 0
#define LIGHT_SENSOR_MEAS_RATE_100MS 1
#define LIGHT_SENSOR_MEAS_RATE_200MS 2
#define LIGHT_SENSOR_MEAS_RATE_500MS 3
#define LIGHT_SENSOR_MEAS_RATE_1000MS 4
#define LIGHT_SENSOR_MEAS_RATE_2000MS 5

class TelstraM1Device
{
public:
	TelstraM1Device(TelstraM1Interface *shield);
	~TelstraM1Device();
	
	void getIMEI(char* resultBuffer);
	void getIP(char* resultBuffer);
	void getTime(char* resultBuffer);
	bool isBG96Connected();
	bool isSIMReady();
	bool isPDPContextActive();
	bool isCellularSystemReady();
	void waitUntilCellularSystemIsReady();
	
	// Reading and storage of IoT Platform credentials
	void writeCredentials(const char *deviceID,const char *deviceTenant,const char *deviceUsername,const char *devicePassword);
	void readCredentials(char *deviceID,char *deviceTenant,char *deviceUsername,char *devicePassword);
	void clearCredentials();
	void resetModem();

	int getBatteryStatus();
	int getBatteryStateOfCharge();
	void getTemperature(char *temperature);
	void getLightLevel(char *lightLevel);
	void getLightSensorState(char* resultBuffer, int len);
	bool isLightSensorDataValid();
	bool isLightSensorDataNew();
	void getLightSensorParams(char* resultBuffer, int len);
	uint8_t getLightSensorGain();
	uint8_t getLightSensorIntegrationTime();
	uint8_t getLightSensorMeasurementRate();
	void setLightSensorParams(uint8_t gain, uint8_t int_rate, uint8_t meas_rate);
	void updateRTCFromNetwork(char* resultBuffer);
	
private:
	TelstraM1Interface *shield;
	int getStatus(char* resultBuffer, int length);

};

#endif // TelstraM1Device_H
