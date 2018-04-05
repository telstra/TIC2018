#ifndef GPS_H
#define GPS_H

#include "Arduino.h"
#include "TelstraM1Device.h"
#include "reporting.h"

#define GPS_ENABLED 1
#define GPS_DISABLED 0

#define ERROR_RESP -1
#define GPS_OK 0

typedef struct {
	char UTC_time[12];
	char UTC_date[8];
	char latitude[12];
	char longitude[12];
	char precision[6];
	char altitude[10];
	uint8_t positioning_mode;
	char cog[8];
	char speed_kmh[8];
	char speed_knot[8];
	uint8_t nsat;	
} GPS_Location;

class GPS{
  private:
    int performTask(uint8_t command);
    char sharedBuffer[TX_BUFFER_SIZE];
    TelstraM1Interface* commsif;
    GPS_Location location;
  public:
  	GPS(TelstraM1Interface *commsif);

    bool enable();
    bool disable();
    int8_t getStatus();
    GPS_Location * get_Location();
};


#endif /* GPS_H */
