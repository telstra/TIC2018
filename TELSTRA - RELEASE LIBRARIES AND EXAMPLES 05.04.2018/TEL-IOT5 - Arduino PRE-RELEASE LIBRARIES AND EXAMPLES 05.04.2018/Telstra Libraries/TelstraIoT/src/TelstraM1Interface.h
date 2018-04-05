#ifndef TelstraM1Interface_H
#define TelstraM1Interface_H

#include <stdint.h>
#include <string.h>
#include <SPI.h>
#include "Arduino.h"
#include "packet_utils.h"
#include "reporting.h"

#define G55_CS       SS1                    // G55 chip CS pin.
#define G55_SPI_PORT SPI1                   // What SPI port is G55 on?
#define G55_RESET 4
#define G55_DATA_RDY 2

#if defined(__ARM__) || defined(ARDUINO_ARCH_SAMD)
  #define REGTYPE uint32_t
  #define G55_SPI_SPEED 4000000
#else
  #define REGTYPE uint8_t
  #define G55_SPI_SPEED 2000000
#endif

#define BUSY -4

#define TX_BUFFER_SIZE 2048
#define RX_BUFFER_SIZE 2048

#define TELSTRAIOT_STATUS_OK 0
#define TELSTRAIOT_STATUS_ERROR -1
#define TELSTRAIOT_STATUS_TIMEOUT -2

#define SPI_TIMEOUT_MS 500

class TelstraM1Interface
{
public:
	TelstraM1Interface();
	~TelstraM1Interface();

	void begin();
	void end();
	
	bool MessageAvailable();
	int sendCommand(char buffer[], int length);
	int getResponse(char buffer[], int length, int timeout);
	void resetCellularSystem();
	void initTimedOut(unsigned long _waitTime);
	int processTimedOut();

private:
	int chipSelectPin;
	int dataReadyPin;
	int g55_resetPin;
	unsigned long startTime;
	unsigned long waitTime;
	SPIClass *_spi;
	
	int waitResponse(int timeout);
	int spiRead(char buf[], uint16_t len);
	int spiWrite(char buf[], uint16_t len);
};

#endif // TelstraM1Interface_H
