#include "telstraiot.h"



/* ############################## HttpResponse ############################## */

HttpResponse::HttpResponse(Connection4G* conn, char* jsonElement, int elementBufferLength, char* jsonValue, int valueBufferLength) {
	Serial.println("Waiting for response...");
	this->conn = conn;

	for(int i=0;i<HTTP_RESPONSE_LENGTH;i++)
	{
		responseContent[i] = 0;
	}
	responsePtr = 0;


	responseLength = conn->TCPRead(responseContent,HTTP_RESPONSE_LENGTH);

	if(responseLength>=0)
	{
		Serial.println("Response received!");
		/*Serial.println(responseLength);
		Serial.println("Response content: ");
		for(int i=0;i<responseLength;i++)
		{
			Serial.print((char)responseContent[i]);
		}
		Serial.println();*/


		readHeader(NULL);

		if(findJsonElementContaining("control", jsonElement, elementBufferLength)) {

			readJSONValue(jsonValue, valueBufferLength);
		} else {
			jsonValue[0] = '\0';
			jsonElement[0] = '\0';
		}

	} else {
		Serial.println("Timeout - no response received");
	}

}

HttpResponse::HttpResponse(Connection4G* conn, const char* jsonPath, char* buffer, const int bufferLength, char* httpDateBuffer) {
	Serial.println("Waiting for response...");

	this->conn = conn;

	for(int i=0;i<HTTP_RESPONSE_LENGTH;i++)
	{
		responseContent[i] = 0;
	}
	responsePtr = 0;

	responseLength = conn->TCPRead(responseContent,HTTP_RESPONSE_LENGTH);

	if(responseLength>=0)
	{
		Serial.println("Response received!");
		/*Serial.println(responseLength);
		Serial.println("Response content: ");
		for(int i=0;i<responseLength;i++)
		{
			Serial.print((char)responseContent[i]);
		}
		Serial.println();*/



		readHeader(httpDateBuffer);

		if(jsonPath != NULL && buffer != NULL && bufferLength > 0) {
			if(locateJsonValue(jsonPath)) {
				readJSONValue(buffer, bufferLength);
			} else {
				buffer[0] = '\0';
			}
		}
	} else {
		Serial.println("Timeout - no response received");
	}


}

char HttpResponse::readByte()
{
	return responseContent[responsePtr++];
}





bool HttpResponse::findJsonElementContaining(const char* substring, char* buffer, int bufferLength) {
    unsigned int matchIndex = 0;
    int elementIndex = 0;
    char c;
    unsigned long prevTime = millis();
    if (!readToElementStart(c, prevTime)) {
        return false;
    }

    while (true) {
        c = readByte();
        if (c > 0) {

            prevTime = millis();

            if (strlen(substring) > matchIndex) {
                if (substring[matchIndex] == c) {
                    matchIndex++;
                }
            }
            if(c == '"') {
                // end of element
                if (strlen(substring) == matchIndex) {
                    // found it!
                    if(elementIndex < bufferLength) {
                        buffer[elementIndex++] = '\0';
                    } else {
                        buffer[bufferLength - 1] = '\0';
                    }
                    return true;
                } else if(readToElementStart(c, prevTime)) {
                    // start next element
                    matchIndex = 0;
                    elementIndex = 0;
                    continue;
                } else {
                    // end of response, or response not received
                    buffer[0] = '\0';
                    return false;
                }

            }
            if(elementIndex < bufferLength) {
                buffer[elementIndex++] = c;
            }
        } else {
            if ((millis() - prevTime) >= 2000) {
                return false;
            }
        }
    }
}

bool HttpResponse::readToElementStart(char &c, unsigned long &prevTime) {
    do {
        tryagain:
        c = readByte();
        if (c > 0) {

            prevTime = millis();
        } else {
            if((millis() - prevTime) >= 2000) {
                return false;
            }
        }
    } while(c != '{' && c != ',');

    // one more char (must be quote - ")
    while(true) {
        c = readByte();
        if (c > 0) {
            prevTime = millis();

            if(c != '"') {
                goto tryagain;
            }
            return true;
        } else {
            if((millis() - prevTime) >= 2000) {
                return false;
            }
        }
    }
}



void HttpResponse::readHeader(char* httpDateBuffer) {

    readHttpCode();

    if(httpDateBuffer != NULL) {
        readDate(httpDateBuffer);
    }

}

void HttpResponse::readHttpCode() {
    char buff[20];
    buff[19] = '\0';
    // read HTTP code
	Serial.println("Reading HTTP code...");
    char c;
    int i=0;
    do {
        c = readByte();
        if(c > 0) {

            buff[i++] = c;
        }
    } while(i < 19 && buff[i-1] != '\r\n');

    // rfc2616:
    // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    char* code = strstr(buff, " ");
    code++;
    httpCode = atoi(code);
	Serial.println();
	Serial.print("HTTP code: ");
	Serial.println(httpCode);
}

int HttpResponse::readDate(char* httpDate) {
    const char* dateHeader = "Date:";
    unsigned int i = 0; // character index that matched
    char c;
    char prev;
    unsigned long prevTime = millis();
    int exitcode = 0;
    while(i < 5) {
        c = readByte();
        if(prev == '\r\n' && c == prev) {
            exitcode = -1; // not found
            break; // body started
        }
        if (c > 0) {

            prevTime = millis();
            if (c != dateHeader[i++]) {
                // not matching any letter
                i=0;
            }
        } else {
            if((millis() - prevTime) >= 2000) {
                exitcode = -3; // timeout
            }
        }
    }
    if(i == strlen(dateHeader)) { // found date header
        // skip day name, e.g. " Tue, " - precisely we wait for second space
        for(int j = 0; j < 2;) {
            c = readByte();

            if(c == ' ') {
                j++;
            }
        }

        // read the date
        // header: 12 Feb 2013 08:01:26 GMT
        // target: 2013-02-12T08:01:26.000Z
        i = 0; // reusing
        while(c != '\r\n') {
            c = readByte();
            if(c > 0) {

                prevTime = millis();
                switch(i) {
                    // day1
                case 0:
                    httpDate[8] = c;
                    break;
                    // day2
                case 1:
                    httpDate[9] = c;
                    break;
                    // month starts
                case 3:
                    // set helper (temp) chars
                    if(c == 'J')
                        httpDate[5] = httpDate[6] = '1'; // Jan or Jun
                    else if(c == 'M')
                        httpDate[5] = httpDate[6] = '2'; // Mar
                    else
                        httpDate[5] = httpDate[6] = '0'; // other
                    break;
                case 4:
                    if(httpDate[5] == '1') { // Jan or Jun
                        if(c == 'a') { // Jan
                            httpDate[5] = '0';
                            httpDate[6] = '1';
                        } else { // Jun
                            httpDate[5] = '0';
                            httpDate[6] = '6';
                        }
                    }
                    break;
                    // month ends
                case 5:
                    chooseMonth(c, httpDate);
                    break;
                    // year1
                case 7:
                    httpDate[0] = c;
                    break;
                    // year2
                case 8:
                    httpDate[1] = c;
                    break;
                    // year3
                case 9:
                    httpDate[2] = c;
                    break;
                    // year4
                case 10:
                    httpDate[3] = c;
                    break;
                    // hour1
                case 12:
                    httpDate[11] = c;
                    break;
                    // hour2
                case 13:
                    httpDate[12] = c;
                    break;
                    // min1
                case 15:
                    httpDate[14] = c;
                    break;
                    // min2
                case 16:
                    httpDate[15] = c;
                    break;
                    // sec1
                case 18:
                    httpDate[17] = c;
                    break;
                    // sec2
                case 19:
                    httpDate[18] = c;
                    break;
                }
                i++;
            } else {
                if((millis() - prevTime) >= 2000) {
                    return -3; // timeout
                }
            }
        }
        httpDate[4] = httpDate[7] = '-';
        httpDate[13] = httpDate[16] = ':';
        httpDate[10] = 'T';
        httpDate[19] = '.';
        httpDate[20] = httpDate[21] = httpDate[22] = '0';
        httpDate[23] = 'Z';
        httpDate[24] = '\0';

        return 1;
    } else {
        return exitcode;
    }
}

void HttpResponse::chooseMonth(char c, char* httpDate) {
    /* 3rd letter (month):
        n - Jan, Jun (done)
        b - Feb
        r - Mar (2), Apr (0)
        y - May
        l - Jul
        g - Aug
        p - Sep
        t - Oct
        v - Nov
        c - Dec
    */
    if(httpDate[5] != httpDate[6])
        return; // already done (Jan or Jun)

    switch(c) {
    case 'b' :
        httpDate[5] = '0';
        httpDate[6] = '2';
        break;
    case 'r' :
        if(httpDate[5] == '2')  { // Mar
            httpDate[5] = '0';
            httpDate[6] = '3';
        } else {
            httpDate[5] = '0';
            httpDate[6] = '4';
        }
        break;
    case 'y' :
        httpDate[5] = '0';
        httpDate[6] = '5';
        break;
    case 'l' :
        httpDate[5] = '0';
        httpDate[6] = '7';
        break;
    case 'g' :
        httpDate[5] = '0';
        httpDate[6] = '8';
        break;
    case 'p' :
        httpDate[5] = '0';
        httpDate[6] = '9';
        break;
    case 't' :
        httpDate[5] = '1';
        httpDate[6] = '0';
        break;
    case 'v' :
        httpDate[5] = '1';
        httpDate[6] = '1';
        break;
    case 'c' :
        httpDate[5] = '1';
        httpDate[6] = '2';
        break;
    }
}

void HttpResponse::skipInnerJsonObject() {
    int depth = 1;
    char c;
    while (depth > 0) {
        c = readByte();

        if (c == '{')
            depth++;
        else if (c == '}')
            depth--;
    }
}

void HttpResponse::readJSONValue(char* buffer, const int bufferLength) {
    bool json = isJSONObject(buffer);
    // read to buffer until " char
    int i = json ? 1 : 0;
    int jsonDepth = 1;
    char c;
    while (i < bufferLength) { // max i on exit = bufferLength
        c = readByte();
        if (c > 0) {

            if(c != '\\') { // TODO handle double backlash also (ignore once)
                if (!json && c == '"') {
                    break;
                }
                buffer[i++] = c;
                if (json) {
                    if(c == '{')
                        jsonDepth++;
                    else if (c == '}' && --jsonDepth == 0) {
                        break;
                    }
                }
            }
        }
    }
    if ( i == bufferLength) {
        Serial.println(F("OVERFLOW"));
        buffer[0] = '\0';
    } else {
        buffer[i++] = '\0'; // max i after instruction = bufferLength
    }
}

bool HttpResponse::isJSONObject(char* buffer) {
    char c;
    while(true) {
        c = readByte();

        if (c == ':') {
            break;
        }
    }
    // next char is { or "
    while (true) {
        c = readByte();

        if (c == '{') {
            buffer[0] = c;
            return true; // JSON object
        } else if (c == '"') {
            return false; // string object
        }
    }
}

bool HttpResponse::locateJsonValue(const char* jsonPath) {
    unsigned int i = 0; // character index that matched
    int pieceBeginning=0;
    char c;
    unsigned long prevTime = millis();
    while(i < strlen(jsonPath)) {
        c = readByte();
        if (c > 0) {
            prevTime = millis();
            if (c == jsonPath[i]) {
                if (jsonPath[++i] == '/') {
                    i++;
                    pieceBeginning = i;

                    // read to open inner object
                    while (c != '{') {
                        c = readByte();
                        if (c > 0) {
                            prevTime = millis();
                        } else {
                            if((millis() - prevTime) >= 2000)
                                goto timeout;
                        }
                    }
                }
            } else {
                // not matching any letter
                i = pieceBeginning;
                if (i != 0) {
                    // at least one outer object already found
                    // but inner object char not yet matching
                    if (c == '{') { // entering inner object, we want to skip it entirely...
                        skipInnerJsonObject();
                    }
                }
            }
        } else {
            if((millis() - prevTime) >= 2000)
			{
                goto timeout;
			}
        }
    }
    return i == strlen(jsonPath);
timeout:
    return false;
}

void HttpResponse::waitForEndOfResponse() {
    char c;

    unsigned long prevTime = millis();
    while (true) {
        c = readByte();
        if (c > 0) {

            prevTime = millis();
        } else {
            if((millis() - prevTime) >= 2000)
                break;
        }
    }
}

int HttpResponse::getHttpCode() {
    return httpCode;
}


/* ############################## HttpRequest ############################## */
HttpRequest::HttpRequest(Connection4G* conn, const char* host, const char* path, const int port, const char* type)
{
	this->conn = conn;
	for(int i=0;i<HTTP_REQUEST_LENGTH;i++)
	{
		requestContent[i] = 0;
	}
	requestLength = 0;

	write(type);
    write(" ");
    write(path);
    write(HTTP_HEADER_FINISH);
    write(host);
}

HttpRequest::HttpRequest(Connection4G* conn)
{
	for(int i=0;i<HTTP_REQUEST_LENGTH;i++)
	{
		requestContent[i] = 0;
	}
	requestLength = 0;
	this->conn = conn;

}

void HttpRequest::write(char* content)
{
	//Serial.println(content);
	strcpy(requestContent+requestLength,content);
	requestLength+=strlen(content);

	/*
	int i=0;
	while(true)
	{
		requestContent[requestLength] = content[i];
		requestLength++;
		if(content[i]=='\0'||requestLength>=HTTP_REQUEST_LENGTH)
		{
			break;
		}
	}*/
}

void HttpRequest::write(const char* content)
{
	//Serial.println(content);
	strcpy(requestContent+requestLength,content);
	requestLength+=strlen(content);

/*	int i=0;
	while(true)
	{
		requestContent[requestLength] = content[i];
		requestLength++;
		if(content[i]=='\0'||requestLength>=HTTP_REQUEST_LENGTH)
		{
			break;
		}
	}*/
}

void HttpRequest::write(int val)
{
	//Serial.println(val);
	char buf[10];
	snprintf(buf, sizeof(buf),"%d",val);
	write(buf);
}


void HttpRequest::authorization(const char* authorization) {
    write("\r\nAuthorization: Basic ");
    write(authorization);
}

void HttpRequest::applicationKey(const char* applicationKey) {
    write("\r\nX-Cumulocity-Application-Key: ");
    write(applicationKey);
}

void HttpRequest::writeMeasurementContentType() {
    write("\r\nContent-Type: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_MEASUREMENT_09);
    write(TELSTRAIOT_CT_SUFFIX);
    write("\r\nAccept: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_MEASUREMENT_09);
    write(TELSTRAIOT_CT_SUFFIX);
}

void HttpRequest::writeManagedObjectContentType() {
    write("\r\nContent-Type: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_MANAGED_OBJECT_09);
    write(TELSTRAIOT_CT_SUFFIX);
}

void HttpRequest::writeManagedObjectReferenceContentType() {
    write("\r\nContent-Type: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_MANAGED_OBJECT_REF_09);
    write(TELSTRAIOT_CT_SUFFIX);
}

void HttpRequest::writeAlarmContentType() {
    write("\r\nContent-Type: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_ALARM_09);
    write(TELSTRAIOT_CT_SUFFIX);
    write("\r\nAccept: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_ALARM_09);
    write(TELSTRAIOT_CT_SUFFIX);
}

void HttpRequest::writeOperationContentType() {
    write("\r\nContent-Type: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_OPERATION_09);
    write(TELSTRAIOT_CT_SUFFIX);
}

void HttpRequest::writeManagedObjectAccept() {
    write("\r\nAccept: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_MANAGED_OBJECT_09);
    write(TELSTRAIOT_CT_SUFFIX);
}

void HttpRequest::writeOperationCollectionAccept() {
    write("\r\nAccept: ");
    write(TELSTRAIOT_CT_PREFIX);
    write(TELSTRAIOT_CT_OPERATION_COLLECTION_09);
    write(TELSTRAIOT_CT_SUFFIX);
}

void HttpRequest::startContent(int contentLength) {
    write("\r\nContent-Length: ");
    write(contentLength);
    write("\r\n\r\n");
}

void HttpRequest::print() {
	Serial.println();
 	Serial.println("=============================");
	Serial.println();
	for(int i=0;i<HTTP_REQUEST_LENGTH;i++)
	{
		Serial.print(requestContent[i]);
	}
 	Serial.println();
 	Serial.println("=============================");
    Serial.println();
}


HttpResponse* HttpRequest::execute(char* jsonElement, int elementBufferLength, char* jsonValue, int valueBufferLength) {
    write("\r\n\r\n\0");

	Serial.println("Sending command...");
	// wait for response
	conn->TCPWrite(requestContent,requestLength);
	delay(500);
	//Serial.println("Getting response...");


    return new HttpResponse(conn, jsonElement, elementBufferLength, jsonValue, valueBufferLength);
}

HttpResponse* HttpRequest::execute(const char* jsonPath, char* buffer, int bufferLength, char* timeBuffer) {
    write("\r\n\r\n\0");

	Serial.println("Sending command...");
	conn->TCPWrite(requestContent,requestLength);
	delay(500);
	//Serial.println("Getting response...");

 	// wait for response

    return new HttpResponse(conn, jsonPath, buffer, bufferLength, timeBuffer);
}

HttpResponse* HttpRequest::execute(char* timeBuffer) {
	//print();
    return execute(NULL, NULL, 0, timeBuffer);
}




/* ############################## TelstraIoT ############################## */
static const char* TELSTRAIOT_ARDUINO_TYPE = "Arduino";


void TelstraIoT::Construct(const char* _host, const int _port, const char* _tenantId, const char* _user, const char* _password, const char* _applicationKey, Connection4G* conn, TelstraM1Device* shield) {
    credentialsSet = true;
	host = _host;
    port = _port;
    tenantId = _tenantId;
    user = _user;
    password = _password;
    applicationKey = _applicationKey;
	this->conn = conn;
	this->shield = shield;
}

TelstraIoT::TelstraIoT(Connection4G* conn, TelstraM1Device* shield) {
	credentialsSet = false;
	this->conn = conn;
	this->shield = shield;
}

TelstraIoT::TelstraIoT(const char* host, const int port, const char* tenantId, const char* user, const char* password, const char* applicationKey, Connection4G* conn, TelstraM1Device* shield) {
    Construct(host, port, tenantId, user, password, applicationKey, conn, shield);
}

TelstraIoT::TelstraIoT(const char* host, const char* tenantId, const char* user, const char* password, const char* applicationKey, Connection4G* conn, TelstraM1Device* shield) {
    Construct(host, 443, tenantId, user, password, applicationKey, conn, shield);
}

TelstraIoT::~TelstraIoT() {}

// ------------------ private

void TelstraIoT::writeAuth(HttpRequest* request) {
    //<realm>/<user name>:<password>

    int input_size = strlen(tenantId) + strlen(user) + strlen(password) + 2;
    int output_size = ((input_size * 4) / 3) + (input_size / 96) + 6;
    char plain_auth[input_size];
    char encoded_auth[output_size];

    strcpy(plain_auth, tenantId);
    strcat(plain_auth, "/");
    strcat(plain_auth, user);
    strcat(plain_auth, ":");
    strcat(plain_auth, password);

    base64_encode(encoded_auth, plain_auth, input_size);


    request->authorization(encoded_auth);
}

char* TelstraIoT::prepareFragment(const char** fragmentValues, const int noValues){
    //"<fragmentName>":["<fragmentValue1>","<fragmentValue2>, ..."]
    int size = 2;//[]
    for (int i = 0; i < noValues; i++){
        size += strlen(fragmentValues[i]) + 2;//size + quotation marks
    }
    if(noValues > 0){
        size += (noValues - 1);//colons
    }
    size++;//one extra char at the end
    char* result = (char*) malloc(size * sizeof(char));

    strcpy(result, "[");
    for (int i = 0; i < noValues; i++){
        strcat(result, "\"");
        strcat(result, fragmentValues[i]);
        strcat(result, "\"");
        if(i < noValues - 1){
            strcat(result,  ",");
        }
    }
    strcat(result, "]");

    return result;
}

HttpRequest* TelstraIoT::prepareRegistrationRequest(){

    //HttpClient client(mod);
    HttpRequest* request = new HttpRequest(conn, host, MANAGED_OBJECT_PATH, 443, "POST");//client.post(host, MANAGED_OBJECT_PATH);
    writeAuth(request);
    request->writeManagedObjectContentType();
    request->writeManagedObjectAccept();
    return request;
}

int TelstraIoT::registerInPlatform(const char* type, const char* name, char* id, const int idLength, const char** supportedOperations, const int nSupportedOperations, const char** supportedMeasurements, const int nSupportedMeasurements) {

    HttpRequest* request = prepareRegistrationRequest();

    char* operationsFragment = prepareFragment(supportedOperations, nSupportedOperations);
    int operationsFragmentLength = strlen(operationsFragment);

    char* measurementFragment = prepareFragment(supportedMeasurements, nSupportedMeasurements);
    int measurementFragmentLength = strlen(measurementFragment);

    request->startContent(128 + strlen(name) + strlen(type) + operationsFragmentLength + measurementFragmentLength);

    request->write("{\"name\":\"");
    request->write(name);
    request->write("\",\"type\":\"");
    request->write(type);
    request->write("\",\"com_cumulocity_model_Agent\":{}");
    request->write(",\"c8y_IsDevice\":{}");
    request->write(",\"c8y_SupportedOperations\":");

    request->write(operationsFragment);
    free(operationsFragment);
    request->write(",\"c8y_SupportedMeasurements\":");

    request->write(measurementFragment);
    free(measurementFragment);
    request->write("}");
    request->write("\r\n\r\n");

	//request->print();

	delete request;
	HttpResponse* response = request->execute("id", id, idLength);
    int exitcode = response->getHttpCode();
    delete response;
    if(exitcode == 201) {
        exitcode = 1;
		Serial.println("Success! Saving credentials to IoT shield");
		strcpy(deviceId,id);

		writeCredentials(deviceId,tenantId,user,password);
		Serial.println("Credentials saved.");
		Serial.println("Note: saved credentials can be cleared using clearCredentials()");

    } else if (exitcode == 0) {
        exitcode = -2;
    } else {
        exitcode *= -1;
    }
    return exitcode;

}

int TelstraIoT::linkID(char* intId, char* extId) {
	char path[80];
	strcpy (path,"/identity/globalIds/");
	strcat (path,intId);
	strcat (path,"/externalIds");

    HttpRequest* request = new HttpRequest(conn, host, path, 443, "POST");
    writeAuth(request);

    request->write("\r\nContent-Type: ");
    request->write(TELSTRAIOT_CT_PREFIX);
    request->write("externalId");
    request->write(TELSTRAIOT_CT_SUFFIX);
    request->write("\r\nAccept: ");
    request->write(TELSTRAIOT_CT_PREFIX);
    request->write("externalId");
    request->write(TELSTRAIOT_CT_SUFFIX);

    int contentLength = 17 + strlen(extId);
	//Serial.print("Content length: ");
	//Serial.println(contentLength);


    request->startContent(contentLength);
    request->write("{\"type\":\"c8y_Serial\",\"externalId\":\"");
    request->write(extId);
    request->write("\"}");


	//request->print();
    HttpResponse* response = request->execute();
    delete request;
    int exitcode = response->getHttpCode();
	delete response;
    if (exitcode == 0) {
		Serial.println("No response received!");
        exitcode = -2;
    } else if(exitcode == 201) {
		Serial.println("Received valid HTTP response!");
        exitcode = 1;
    } else if (exitcode > 0) {
 		Serial.print("Received invalid response code - HTTP ");
 		Serial.println(exitcode);
       exitcode *= -1;
    }
	//Serial.println("Response deleted!");
    return exitcode;
}


int TelstraIoT::registerDevice(const char* name, char* buffer, int bufferSize) {
    const char* supportedOperations[0];
    const char*  supportedMeasurements[0];
    return registerDevice(name, buffer, bufferSize, supportedOperations, 0, supportedMeasurements, 0);
}

int TelstraIoT::registerDevice(const char* name, char* buffer, int bufferSize,
        const char** supportedOperations, const int nSupportedOperations,
        const char** supportedMeasurements, const int nSupportedMeasurements) {
    Serial.println();
    int exitcode;


    exitcode = registerInPlatform(TELSTRAIOT_ARDUINO_TYPE, name, buffer, bufferSize,
        supportedOperations, nSupportedOperations, supportedMeasurements, nSupportedMeasurements);
    if(exitcode > 0) {
        Serial.println(buffer);
        exitcode = 1;
    }
    return exitcode;
}

int TelstraIoT::registerDevice(const char* name, char* buffer, int bufferSize, const char** supportedMeasurements, const int nSupportedMeasurements) {
    Serial.println();
    int exitcode;

    isDeviceRegistered((const char *)buffer,NULL,name);
	const char** ops = NULL;
    exitcode = registerInPlatform(TELSTRAIOT_ARDUINO_TYPE, name, buffer, bufferSize,ops, 0, supportedMeasurements, nSupportedMeasurements);
    if(exitcode > 0) {
        Serial.println(buffer);
        exitcode = 1;
    }
    return exitcode;
}


int TelstraIoT::isDeviceRegistered(const char* id, const char* type, const char* name) {
    HttpRequest* request = new HttpRequest(conn);

    // custom path writing
    request->write("GET ");
    request->write("/identity/externalIds");
    request->write("/");
    request->write(id);
    request->write(HTTP_HEADER_FINISH);
    request->write(host);

    writeAuth(request);
    request->writeManagedObjectAccept();
    request->write("\r\n\r\n");

    char registeredName[strlen(name) + 1];
    HttpResponse* response = request->execute("name", registeredName, strlen(name) + 1);
	//request->print();
    delete request;
    int exitcode = response->getHttpCode();
 	delete response;
   if (exitcode == 200) {
        if (strcmp(registeredName, name) == 0) {
            exitcode = 1;
        } else {
            exitcode = -1;
        }
    } else if (exitcode == 0) {
        exitcode = -2;
    } else if (exitcode == 404) {
        exitcode = -1;
    } else {
        exitcode *= -1;
    }
    return exitcode;
}


int TelstraIoT::raiseAlarm(const char* type, const char* status, const char* severity, const char* text) {
	char time[50];
	shield->getTime(time);

    return raiseAlarm(type, status, severity, time, text);
}

int TelstraIoT::raiseAlarm(const char* type, const char* status, const char* severity, const char* time, const char* text) {
    HttpRequest* request = new HttpRequest(conn, host, "/alarm/alarms", 443, "POST");
	//Serial.println("Request created");
    writeAuth(request);
    request->writeAlarmContentType();

 	//Serial.println("Auth written");

    int contentLength = 76 + strlen(deviceId) + strlen(type) + strlen(status) + strlen(severity) + strlen(time) + strlen(text);
	//Serial.print("Content length: ");
	//Serial.println(contentLength);


    request->startContent(contentLength);
    request->write("{\"source\":{\"id\":\"");
    request->write(deviceId);
    request->write("\"},\"type\":\"");
    request->write(type);
    request->write("\",\"status\":\"");
    request->write(status);
    request->write("\",\"severity\":\"");
    request->write(severity);
    request->write("\",\"time\":\"");
    request->write(time);
    request->write("\",\"text\":\"");
    request->write(text);
    request->write("\"}");

	//request->print();
    HttpResponse* response = request->execute();
    delete request;
    int exitcode = response->getHttpCode();
	delete response;
    if (exitcode == 0) {
		Serial.println("No response received!");
        exitcode = -2;
    } else if(exitcode == 201) {
		Serial.println("Received valid HTTP response!");
        exitcode = 1;
    } else if (exitcode > 0) {
 		Serial.print("Received invalid response code - HTTP ");
 		Serial.println(exitcode);
       exitcode *= -1;
    }
	//Serial.println("Response deleted!");
    return exitcode;
}

int TelstraIoT::sendMeasurement(const char* type, const char* fragmentName, const char* measurementName, const long &mValue, const char* mUnit) {
	char time[50];
	shield->getTime(time);
	char measurementString[12];
	sprintf(measurementString,"%ld",mValue);
    return sendMeasurement(type, time, fragmentName, measurementName, measurementString, mUnit);
}

HttpRequest* TelstraIoT::startMeasurementRequest(const char* type, const char* time, const char* measurementName, const char* mUnit) {
	//Serial.println("Creating HTTP request");
    HttpRequest* request = new HttpRequest(conn, host, "/measurement/measurements", 443, "POST");
	//Serial.println("	Done!");

    writeAuth(request);
    request->writeMeasurementContentType();

    return request;
}

int TelstraIoT::sendMeasurement(const char* type, const char* time, const char* fragmentName, const char* measurementName, const long &mValue, const char* mUnit) {
	char measurementString[12];
	sprintf(measurementString,"%ld",mValue);
    return sendMeasurement(type, time, fragmentName, measurementName, measurementString, mUnit);
}

int TelstraIoT::sendMeasurement(const char* type, const char* fragmentName, const char* measurementName, char* measurementString, const char* mUnit) {
	char time[50];
	shield->getTime(time);
    return sendMeasurement(type, time, fragmentName, measurementName, measurementString, mUnit);
}

int TelstraIoT::sendMeasurement(const char* type, const char* time, const char* fragmentName, const char* measurementName, char* measurementString, const char* mUnit) {
 	//Serial.println("Entering sendMeasurement()");
    HttpRequest* request = startMeasurementRequest(type, time, measurementName, mUnit);
	//Serial.println("Request created");

    //char* mValueStr = (char*) malloc(12*sizeof(char));
    //ltoa(mValue, mValueStr, 10);

	int contentLength = 69 + strlen(deviceId) + strlen(time) + strlen(type) + strlen(fragmentName) + strlen(measurementName) + strlen(measurementString) + strlen(mUnit);


     //  Write content:
     // {
     // "time" : "2012-12-31T23:59:59.999Z",
     // "type" : "ArduinoMeasurement",
     // "source" : { "id": "XXX" },
     // "measurementName": { "value": YYY, "unit": "ZZZ" }
     // }



	Serial.println("Starting content");

    request->startContent(contentLength);
    request->write("{\"source\":{\"id\":\"");
    request->write(deviceId);
    request->write("\"},\"time\":\"");
    request->write(time);
    request->write("\",\"type\":\"");
    request->write(type);
    request->write("\",\"");
    request->write(fragmentName);
    request->write("\":{\"");
    request->write(measurementName);
    request->write("\":{\"value\":");
    request->write(measurementString);
    //free(mValueStr);
    request->write(",\"unit\":\"");
    request->write(mUnit);
    request->write("\"}}}");

	//request->print();
    HttpResponse* response = request->execute();
    delete request;
    int exitcode = response->getHttpCode();
	delete response;
    if (exitcode == 0) {
		Serial.println("No response received!");
        exitcode = -2;
    } else if(exitcode == 201) {
		Serial.println("Received valid HTTP response!");
        exitcode = 1;
    } else if (exitcode > 0) {
 		Serial.print("Received invalid response code - HTTP ");
 		Serial.println(exitcode);
       exitcode *= -1;
    }
	//Serial.println("Response deleted!");
    return exitcode;
}

const char* TelstraIoT::getTime() {

    return currentTime;
}

void TelstraIoT::writeCredentials(const char *deviceId,const char *deviceTenant,const char *deviceUsername,const char *devicePassword) {

    shield->writeCredentials(deviceId,deviceTenant,deviceUsername,devicePassword);
}

void TelstraIoT::readCredentials(char *deviceId,char *deviceTenant,char *deviceUsername,char *devicePassword) {

    shield->readCredentials(deviceId,deviceTenant,deviceUsername,devicePassword);
}

void TelstraIoT::clearCredentials() {

    shield->clearCredentials();
}

void TelstraIoT::setCredentials(const char* _deviceId, const char* _tenantId, const char* _user, const char* _password, const char* _applicationKey) {
    credentialsSet = true;
    for(int i=0;i<strlen(_deviceId);i++)
	{
		deviceId[i] = _deviceId[i];
	}

	tenantId = _tenantId;
    user = _user;
    password = _password;
    applicationKey = _applicationKey;

}

void TelstraIoT::setHost(const char* _host, const int _port) {
	host = _host;
  port = _port;
}


/* ############################## BASE64 ############################## */

void base64_encode(char* output, char const* input, unsigned int in_len) {
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    int out_idx = 0;

    while (in_len--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i < 4) ; i++)
                output[out_idx++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            output[out_idx++] = base64_chars[char_array_4[j]];

        while((i++ < 3))
            output[out_idx++] = '=';
    }
    output[out_idx++] = '\0';
}
