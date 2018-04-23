#ifndef AZURE_IOT_H
#define AZURE_IOT_H

#include "Arduino.h"
#include "sha256.h"
#include "Base64.h"
#include <TimeLib.h> 
#include "TelstraM1Device.h"

class AzureIoT
{
public:
	AzureIoT(const char* azureHost, const char* deviceId, const char* key, int ttl);
	char* printSas();
	void setPostContent(String content);
	char* getPostPacket();
	void newSas(int ttl);
	void init(TelstraM1Device* IoTDevice);

protected:
    String urlEncode(const char* msg);
	const char* getStringValue(String value);

private:
	String generateSas(char* key, String endPointUri);
	void currentTime(char* timeBuffer);
	int getNumber(char *input, int len);

	time_t  sasExpiryTime = 0;
	char* sasToken = new char[0];
	char* post = new char[0];
	const char* azureHost;
	const char* deviceId;
	const char* key;
	String endPoint;
	String contentInfo;
	time_t ttl;
	void setBoardCurrentTime(TelstraM1Device* IoTDevice);
};

#endif /*AZURE_IOT_H*/
