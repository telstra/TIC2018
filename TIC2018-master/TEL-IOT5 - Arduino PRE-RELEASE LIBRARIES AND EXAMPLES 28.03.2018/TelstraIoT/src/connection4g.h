#ifndef CONNECTION4G_H
#define CONNECTION4G_H

#include <stdint.h>
#include <string.h>
#include "packet_utils.h"
#include "str_utils.h"
#include "TelstraM1Interface.h"
#include "Arduino.h"
#include "shieldcommands.h"

#define CONNECTION4G_STATUS_OK     0
#define CONNECTION4G_STATUS_ERROR -1
#define CONNECTION4G_STATUS_TIMEOUT -2



class Connection4G
{
public:
	Connection4G(bool secure, TelstraM1Interface *shield);
	~Connection4G();

	int ping(const char *host, char *responseBuffer);
	int activatePDP(const char *apn, const char *username, const char *password);
	int deactivatePDP();
	int openTCP(const char *host, uint16_t port);
	int closeTCP();
	int TCPRead(char *buffer, uint16_t length);
	int TCPWrite(char *data, uint16_t length);
	int getSignalQuality(char *data);


private:
	bool secure;
	TelstraM1Interface *shield;
	

};

#endif // CONNECTION4G_H








