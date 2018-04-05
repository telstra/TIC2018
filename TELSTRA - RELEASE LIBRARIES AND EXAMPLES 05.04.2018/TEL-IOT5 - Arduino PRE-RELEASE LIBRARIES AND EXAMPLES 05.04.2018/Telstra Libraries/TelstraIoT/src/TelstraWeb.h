
#ifndef TELSTRAWEB_H
#define TELSTRAWEB_H

#include "Arduino.h"
#include "connection4g.h"
#include "TelstraM1Device.h"
#include "telstraiot.h"
#include "reporting.h"

class TelstraWeb{
  private:
    const char* host;
    const char* path;
    int port;
    Connection4G* conn;
	TelstraM1Device* IoTDevice;
    HttpRequest* setupRequest(const char* method);
  public:
    TelstraWeb(Connection4G* conn, TelstraM1Device* IoTDevice);
    void setHost(const char* _host, const char* path ,const int _port);
    int post(const char* data);
};


#endif /* TELSTRAWEB_H */
