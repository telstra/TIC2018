#include "TelstraM1Interface.h"

/*
 * Initialise IoT Shield
 */
 
 TelstraM1Interface::TelstraM1Interface()
{
	this->chipSelectPin = G55_CS;
	this->dataReadyPin = 2;
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
	this->dataReadyPin = 2;
	this->g55_resetPin = G55_RESET;
	
	pinMode(g55_resetPin, OUTPUT);
	digitalWrite(g55_resetPin, LOW); 
	delay(1);
	digitalWrite(g55_resetPin, HIGH); 
	delay(1000);

	this->_spi->begin();

	pinMode(chipSelectPin, OUTPUT);
	digitalWrite(chipSelectPin, HIGH);
	pinMode(dataReadyPin, INPUT);	
}

void TelstraM1Interface::end()
{
	pinMode(chipSelectPin, INPUT);
	this->_spi->end();
}


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
		return 0; // Time out
	}
}


/*
 * Send data to the G55
 */
int TelstraM1Interface::sendCommand(char buffer[], int length)
{
	if(this->dataReadyPin == HIGH) {
		return BUSY;
	}
	else {
		return spiWrite(buffer,length);
	}
}


/*
 * Wait until the G55 has data ready to send
 */
int TelstraM1Interface::waitResponse(int timeout)
{
	int startTime = millis();
	int elapsedTime = 0;
	
	while(elapsedTime<timeout)
	{
		elapsedTime = millis()-startTime;
		if(elapsedTime<0)
		{
			startTime = 0;
		}
		
		if(digitalRead(dataReadyPin) == HIGH)
		{
			return TELSTRAIOT_STATUS_OK;
		}
	}
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
    while (len--) {
      c = *buf;
      buf++;
      this->_spi->transfer(c);
    }
    this->_spi->endTransaction();	

	digitalWrite(chipSelectPin, HIGH);

	return count;
}


