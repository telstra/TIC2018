#ifndef TelstraM1Device_H
#define TelstraM1Device_H

#include <stdint.h>
#include <string.h>
#include "Arduino.h"
#include "g55commands.h"
#include "TelstraM1Interface.h"
#include "packet_utils.h"
#include "reporting.h"

#define CELLULAR_READY_TIMEOUT 30 /* Seconds */
#define M1DEVICE_CMD_TIMEOUT 5000

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
	TelstraM1Device(TelstraM1Interface *commsif);
	~TelstraM1Device();
	
	int getIMEI(char* resultBuffer);
	int getIP(char* resultBuffer);
	int getTime(char* resultBuffer);
	bool isBG96Connected();
	bool isSIMReady();
	bool isPDPContextActive();
	bool isCellularSystemReady();
	int waitUntilCellularSystemIsReady();
	int waitUntilCellularSystemIsReady(int timeout);
	
	// Reading and storage of IoT Platform credentials
	int writeCredentials(const char *deviceID,const char *deviceTenant,const char *deviceUsername,const char *devicePassword);
	int readCredentials(char *deviceID,char *deviceTenant,char *deviceUsername,char *devicePassword);
	int clearCredentials();
	int resetModem();

	// Sensor related functions
	int getBatteryStatus();
	int getBatteryStateOfCharge();
	int getTemperature(char *temperature);
	int getLightLevel(char *lightLevel);
	int getLightSensorState(char* resultBuffer, int len);
	bool isLightSensorDataValid();
	bool isLightSensorDataNew();
	int getLightSensorParams(char* resultBuffer, int len);
	int getLightSensorGain();
	int getLightSensorIntegrationTime();
	int getLightSensorMeasurementRate();
	int setLightSensorParams(uint8_t gain, uint8_t int_rate, uint8_t meas_rate);
	int updateRTCFromNetwork(char* resultBuffer);
	
private:
	TelstraM1Interface *commsif;
	int getStatus(char* resultBuffer, int length);
};

#endif // TelstraM1Device_H
