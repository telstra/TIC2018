#include "TelstraM1Interface.h"

#define MAX_TIMER 4294967295

void TelstraM1Interface::initTimedOut(unsigned long _waitTime){
	this->startTime = millis();
	this->waitTime = _waitTime;
}

int TelstraM1Interface::processTimedOut()
{
	unsigned long now = millis();
	unsigned long duration = 0;

	//Detects timer rollover condition and calculate just the duration
	if (now < this->startTime) { 
		duration = MAX_TIMER - this->startTime;
		duration += now;
	} else {
		duration = now - this->startTime;
	}

	if (duration < this->waitTime)
		return false;
	else
		return true;
}

/*
 * Initialise TelstraM1Interface
 */ 
 TelstraM1Interface::TelstraM1Interface()
{
	this->chipSelectPin = G55_CS;
	this->dataReadyPin = G55_DATA_RDY;
	this->g55_resetPin = G55_RESET;
}

/*
 * Close connection to G55
 */
TelstraM1Interface::~TelstraM1Interface()
{
	// Close connection to G55
	this->_spi->end();
}

void TelstraM1Interface::begin()
{
	this->_spi = &G55_SPI_PORT;
	this->chipSelectPin = G55_CS;
	this->dataReadyPin = G55_DATA_RDY;
	this->g55_resetPin = G55_RESET;
	
	pinMode(g55_resetPin, OUTPUT);
	digitalWrite(g55_resetPin, LOW); 
	delay(10);
	digitalWrite(g55_resetPin, HIGH); 
	delay(1000);

	pinMode(chipSelectPin, OUTPUT);
	digitalWrite(chipSelectPin, HIGH);
	pinMode(dataReadyPin, INPUT);

	this->_spi->begin();	
}

void TelstraM1Interface::resetCellularSystem()
{
	digitalWrite(g55_resetPin, LOW); 
	delay(1);
	digitalWrite(g55_resetPin, HIGH); 
	delay(500);
	reportln(REPORT_L3,"[LOG] resetCellularSystem()[3] : Complete");
}

void TelstraM1Interface::end()
{
	digitalWrite(chipSelectPin, HIGH);
	this->_spi->end();
}

/*
 * Checks if there is a message available from G55, this is a non-blocking funciton
 */
bool TelstraM1Interface::MessageAvailable()
{
	if(digitalRead(dataReadyPin) == HIGH)
		return true;
	else return false;
}

/*
 * Wait until the G55 is ready to send data, then read the data into a buffer
 * Returns STATUS_ERROR if timeout occurs
 */
int TelstraM1Interface::getResponse(char buffer[], int length, int timeout)
{
	if(waitResponse(timeout)==TELSTRAIOT_STATUS_OK)
	{
		return spiRead(buffer, length);
	} else {
		reportln(REPORT_L2, "<SPI Interface:getResponse> - Response Timeout");
		return TELSTRAIOT_STATUS_TIMEOUT; // Time out
	}
}


/*
 * Send data to the G55
 */
int TelstraM1Interface::sendCommand(char buffer[], int length)
{
	reportln(REPORT_L3, "<SPI Interface:sendCommand> - Entering function");
	if(this->dataReadyPin == HIGH) {
		reportln(REPORT_L2, "<SPI Interface:sendCommand> - Cannot send comamand (BUSY)");
		return BUSY;
	} else {
		reportln(REPORT_L3, "<SPI Interface:sendCommand> - Ready to send");
		return spiWrite(buffer,length);
	}
}

/*
 * Wait until the G55 has data ready to send
 */
int TelstraM1Interface::waitResponse(int timeout)
{
	unsigned long startTime =millis();
	while( millis() - startTime < (unsigned long) timeout){
		if(digitalRead(dataReadyPin) == HIGH)
		{
			return TELSTRAIOT_STATUS_OK;
		}
	}
	reportln(REPORT_L2, "<SPI Interface:waitResponse> - Timeout");	
	return TELSTRAIOT_STATUS_TIMEOUT;
}

/*
 * Read data from SPI, if it is available
 */
int TelstraM1Interface::spiRead(char buf[], uint16_t len)
{
	int bytesRead = 0;
	char c;

	digitalWrite(chipSelectPin, LOW);
    this->_spi->beginTransaction(SPISettings(G55_SPI_SPEED, MSBFIRST, SPI_MODE0));
	this->_spi->transfer(0);

	digitalWrite(chipSelectPin, HIGH);
	initTimedOut(SPI_TIMEOUT_MS);

	while(digitalRead(dataReadyPin) == HIGH)
	{
		digitalWrite(chipSelectPin, LOW);
		c = this->_spi->transfer(0x00);
		*buf = c;
		buf++;

		bytesRead++;
		if(bytesRead == len)
		{
			digitalWrite(chipSelectPin, HIGH);			
			return bytesRead;
		}
		digitalWrite(chipSelectPin, HIGH);	

		if (processTimedOut()) { //This means that G55 is not responding
			reportln(REPORT_L2, "<SPI Interface:spiRead> - SPI Read timeout - Resetting...");
			this->_spi->endTransaction();
			return TELSTRAIOT_STATUS_TIMEOUT;
		}
	}
    this->_spi->endTransaction();

	return bytesRead;
}

/*
 * Send data over SPI
 */
int TelstraM1Interface::spiWrite(char buf[], uint16_t len)
{
	char c;
	uint16_t count = len;

	digitalWrite(chipSelectPin, LOW);

    this->_spi->beginTransaction(SPISettings(G55_SPI_SPEED, MSBFIRST, SPI_MODE0));
    initTimedOut(SPI_TIMEOUT_MS);

    while (len--) {
      c = *buf;
      buf++;
      this->_spi->transfer(c);

      if (processTimedOut()) {
      	reportln(REPORT_L2, "<SPI Interface:spiWrite> - SPI Write timeout");
		digitalWrite(chipSelectPin, HIGH);
      	this->_spi->endTransaction();
      	return TELSTRAIOT_STATUS_TIMEOUT;
      }
    }
    this->_spi->endTransaction();	

	digitalWrite(chipSelectPin, HIGH);

	return count;
}


