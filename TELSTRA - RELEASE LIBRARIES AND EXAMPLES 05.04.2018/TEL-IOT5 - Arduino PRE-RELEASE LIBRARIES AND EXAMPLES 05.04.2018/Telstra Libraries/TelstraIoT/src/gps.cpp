#include "gps.h"

GPS::GPS(TelstraM1Interface *commsif){
    this->commsif = commsif;
}

int GPS::performTask(uint8_t command){
    	
	char c_checksum = 0;
    uint16_t c_size = 0;
    int res;

	packet_create_cmd(command, this->sharedBuffer, &c_checksum, &c_size);
	packet_append_ending(this->sharedBuffer, &c_checksum, &c_size);
	
	commsif->sendCommand(this->sharedBuffer, c_size);
	
    res = commsif->getResponse(this->sharedBuffer,TX_BUFFER_SIZE,5000);

    if (res < 0) {
        reportln(REPORT_L2, "<GPS:performTask> - Response, resetting...");
        commsif->resetCellularSystem();
        return ERROR_RESP;
    } else {
        if (packet_get_command(this->sharedBuffer) != CMD_RES_OFFSET + command) {
            reportln(REPORT_L2, "<GPS:performTask> - Unknown response, resetting...");
            commsif->resetCellularSystem();
            return ERROR_RESP;
        } else {
            return GPS_OK;
        }
    }
}

bool GPS::enable(){
    if (performTask(CMD_GPS_ENABLE) != GPS_OK) return false;
    
    if(packet_get_command(this->sharedBuffer)==CMD_GPS_ENABLEACK){
        return true;
    } else {
        return false;
    }
}

bool GPS::disable(){
    if (performTask(CMD_GPS_DISABLE) != GPS_OK) return false;

    if(packet_get_command(this->sharedBuffer)==CMD_GPS_DISABLEACK){
        return true;
    } else {
        return false;
    }
}

int8_t GPS::getStatus(){
    if (performTask(CMD_GPS_GET_STATUS) != GPS_OK) return ERROR_RESP;

    if(packet_get_command(this->sharedBuffer)==CMD_GPS_GET_STATUSACK){    
        int8_t resp = (int8_t)packet_get_numeric_param(this->sharedBuffer,0);
        return resp;
    } else {
        return ERROR_RESP;
    }
}

GPS_Location * GPS::get_Location(){
    if (performTask(CMD_GPS_GET_LOCATION) != GPS_OK) return NULL;
    char tempBuff[10];

    if(packet_get_command(this->sharedBuffer)==CMD_GPS_GET_LOCATIONACK){

        packet_get_param(this->location.UTC_time ,this->sharedBuffer,0);
        packet_get_param(this->location.UTC_date ,this->sharedBuffer,1);
        packet_get_param(this->location.latitude ,this->sharedBuffer,2);
        packet_get_param(this->location.longitude ,this->sharedBuffer,3);
        packet_get_param(this->location.precision ,this->sharedBuffer,4);
        packet_get_param(this->location.altitude ,this->sharedBuffer,5);

        packet_get_param((char *)(&(this->location.positioning_mode)) ,this->sharedBuffer,6);

        packet_get_param(this->location.cog ,this->sharedBuffer,7);
        packet_get_param(this->location.speed_kmh ,this->sharedBuffer,8);
        packet_get_param(this->location.speed_knot ,this->sharedBuffer,9);

        packet_get_param((char *)(&(this->location.nsat)) ,this->sharedBuffer,10);
        return &(this->location);
        
    } else {
        return NULL;
    }
}