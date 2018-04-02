#ifndef JSON_GENERATOR
#define JSON_GENERATOR

#include "ArduinoJson.h"
#include "TelstraM1Device.h"
#include <stdlib.h>

#define BATTERY_STATUS 0x01
#define	BATTERY_STATE_OF_CHARGE 0x02
#define	TEMPARATURE 0x04
#define	LIGHT_LEVEL 0x08



class JsonGenerator
{
	public:
		JsonGenerator(uint8_t parameters_, uint8_t samples);		//Warning: This library can only handle errors of one
		~JsonGenerator();											//sample per sensor. Error Handling under Development

		String printJson();
		uint8_t collectSensorData(TelstraM1Device* IoTDevice);

	private:
		void setBuf();
		void resetBuf();
		String makeJsonData();

		unsigned short c;
		unsigned short count = 0;
		String content;
		uint8_t parameters;
		uint8_t* batteryStatus = nullptr;
		uint8_t* batteryStateOfCharge = nullptr;
		float* temparature = nullptr;
		unsigned int* lightLevel = nullptr;
		bool recollect = false;
		uint8_t errors = 0;




};
#endif /*JSON_GENERATOR*/