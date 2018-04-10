
#ifndef TELSTRAIOT_H
#define TELSTRAIOT_H

#include "Arduino.h"
#include "connection4g.h"
#include "TelstraM1Device.h"
#include "reporting.h"

#define HTTP_REQUEST_LENGTH 1000
#define HTTP_RESPONSE_LENGTH 2000

#define RESPONSE_REQUEST_DELAY 2000

#define CONSOLE_OUTPUT_ENABLE true
#define CONSOLE_OUTPUT_DISABLE false

class HttpResponse {

private:
  int httpCode;
	char responseContent[HTTP_RESPONSE_LENGTH];
	int responseLength;
	int responsePtr;
	Connection4G* conn;

	char readByte();

    void readHeader(char* httpDateBuffer);
    void readHttpCode();
    int readDate(char* httpDateBuffer);
    void chooseMonth(char c, char* httpDate);
    void skipInnerJsonObject();
    bool findJsonElementContaining(const char* substring, char* buffer, int bufferLength);
    bool readToElementStart(char &c, unsigned long &prevTime);
    void readJSONValue(char* buffer, const int bufferLength);
    /**
      * if JSON object, the function writes '{' char to first element of the buffer.
      * Returns true in such case.
      */
    bool isJSONObject(char* buffer);
    bool locateJsonValue(const char* jsonPath);
    void waitForEndOfResponse();
public:
    /**
      * Creates HttpResponse. Remember to use #cleanMemory() method
      * when you are done with value of requested JSON element (see #getRequestedValue() ).
      * Parameters:
      *     jsonPath - path to the JSON element which value needs to be saved
      *     buffer - buffer to save JSON value to
      *     bufferLength - length of the buffer
      *     [httpDateBuffer] - buffer for date (optional)
      * Parameter httpDateBuffer must be at least 23 long! (no check performed)
      */
    HttpResponse(Connection4G* conn, const char* jsonPath, char* buffer, const int bufferLength, char* httpDateBuffer = NULL);
    HttpResponse(Connection4G* conn, char* jsonElement, int elementBufferLength, char* jsonValue, int valueBufferLength);

    /**
      * Returns the HTTP code of the response.
      */
    int getHttpCode();
};






class HttpRequest {
	private:
		char requestContent[HTTP_REQUEST_LENGTH];
		int requestLength;
		Connection4G* conn;

	public:
		HttpRequest(Connection4G* conn, const char* host, const char* path, const int port, const char* type);
		HttpRequest(Connection4G* conn);

		void write(char* content);
		void write(const char* content);
		void write(int val);
		void authorization(const char* authorization);
		void applicationKey(const char* applicationKey) ;
		void writeMeasurementContentType() ;
		void writeManagedObjectContentType() ;
		void writeManagedObjectReferenceContentType() ;
		void writeAlarmContentType() ;
		void writeOperationContentType() ;
		void writeManagedObjectAccept() ;
		void writeOperationCollectionAccept() ;
		void startContent(int contentLength) ;
		void print();

		HttpResponse* execute(const char* jsonPath, char* buffer, int bufferLength, char* timeBuffer = NULL);
		HttpResponse* execute(char* jsonElement, int elementBufferLength, char* jsonValue, int valueBufferLength);
		HttpResponse* execute(char* timeBuffer = NULL);

};



#define RX_FINISHED_STR "RX FINISHED STR"
#define HTTP_HEADER_FINISH " HTTP/1.1\r\nHost: "

// content types
#define TELSTRAIOT_CT_PREFIX "application/vnd.com.nsn.cumulocity."
#define TELSTRAIOT_CT_SUFFIX "+json;ver=0.9;charset=UTF-8"//;ver=0.9;charset=UTF-8
#define TELSTRAIOT_CT_MANAGED_OBJECT_09 "managedObject"
#define TELSTRAIOT_CT_MANAGED_OBJECT_REF_09 "managedObjectReference"
#define TELSTRAIOT_CT_MEASUREMENT_09 "measurement"
#define TELSTRAIOT_CT_OPERATION_COLLECTION_09 "operationCollection"
#define TELSTRAIOT_CT_OPERATION_09 "operation"
#define TELSTRAIOT_CT_ALARM_09 "alarm"

// http paths
#define MANAGED_OBJECT_PATH "/inventory/managedObjects"
#define CHILD_DEVICES_PATH_SUFFIX "/childDevices"
#define IDENTITY_PATH "/identity"
#define OPERATION_PATH_PREFIX "/devicecontrol/operations"
#define GET_OPERATION_PATH_CRITERIA "?pageSize=1&deviceId="
#define GET_OPERATION_PATH_SUFFIX "&status=PENDING"






/** TelstraIoT class is the main access point to the IoT Platform API.
  * All methods that return int have standardized exit codes:\n
  * Exit code | Description
  * -----------|------------
  * 1          | OK
  * 0	       | Object already exists (OK) / operation not found (OK)
  * -1	       | Not found
  * -2	       | Response not received
  * -3	       | Timeout reading response
  * < -200	   | Http error code multiplied by -1
  */
class TelstraIoT {

private:
    const char* host;
    const char* tenantId;
    const char* user;
    const char* password;
    const char* applicationKey;
    int port;

	  char deviceId[10];

    unsigned long lastTimeUpdate;
    char currentTime[50];

	  bool credentialsSet;

    Connection4G* conn;
	  TelstraM1Device* IoTDevice;

    void writeAuth(HttpRequest* request);

   
    int registerInPlatform(const char*,const char*, char* id, const int, const char**, const int, const char**, const int);
    HttpRequest* prepareRegistrationRequest();
    char* prepareFragment(const char** fragmentValues, const int noValues);
    void Construct(const char* _host, const int _port, const char* _tenantId, const char* _user, const char* _password, const char* _applicationKey, Connection4G* conn, TelstraM1Device* IoTDevice);

    HttpRequest* startAlarmRequest();
    HttpRequest* startMeasurementRequest(const char* type, const char* time, const char* measurementName, const char* mUnit);
    HttpRequest* startOperationUpdateRequest(const char* id);
    int getOperationId(char* buffer, int bufferLength);
    int updateOperation(const char* id, const __FlashStringHelper* newStatus);
    void addTime(char* str, unsigned long diff);
    unsigned int daysInMonth(int month, int year);
    bool isLeapYear(int year);
    int isDeviceRegistered(const char*, const char*, const char*);
    int restablishCellularSystem();
	

public:
 /**
	  * Create TelstraIoT instance without credentials
	  * Credentials must be set later using setCredentials()
	  */
    TelstraIoT(Connection4G* conn, TelstraM1Device* IoTDevice);


    /**
      * Creates TelstraIoT instance.
      *     @param host - host of the cumulocity platform in format: [http://]domain.com (without /platform !)
      *     @param port - port of the cumulocity platform API
      *     @param tenantId - for authorization
      *     @param user - for authorization
      *     @param password - for authorization
      *     @param applicationKey - for authorization
      */
    TelstraIoT(const char* host, const int port, const char* tenantId, const char* user, const char* password, const char* applicationKey, Connection4G* conn, TelstraM1Device* IoTDevice);

    /**
      * Creates TelstraIoT instance using default port (443).
      *     @param host - host of the cumulocity platform in format: [http://]domain.com (without /platform !)
      *     @param tenantId - for authorization
      *     @param user - for authorization
      *     @param password - for authorization
      *     @param applicationKey - for authorization
      */
    TelstraIoT(const char* host, const char* tenantId, const char* user, const char* password, const char* applicationKey, Connection4G* conn, TelstraM1Device* IoTDevice);
    virtual ~TelstraIoT();

    /**
      * Sends alarm information to the server.
      *     @param type - type of the alarm
      *     @param status - status of the alarm (active, acknowledged, cleared)
      *     @param severity - severity of the alarm (critical, major, minor, warning)
      *     @param time - time of alarm
      *     @param text - information about the alarm
      *     @return 1 in case of success
      *     @return -2 error reading response (impossible to determine success)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int raiseAlarm(const char* type, const char* status, const char* severity, const char* time, const char* text);

    /**
      * Sends alarm information to the server. Makes additional request to obtain current time.
      *     @param type - type of the alarm
      *     @param status - status of the alarm (active, acknowledged, cleared)
      *     @param severity - severity of the alarm (critical, major, minor, warning)
      *     @param text - information about the alarm
      *     @return 1 in case of success
      *     @return -2 error reading response (impossible to determine success)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int raiseAlarm(const char* type, const char* status, const char* severity, const char* text);

    /**
      * Sends measurement to the server.
      *     @param type - type of measurement
      *     @param time - time of measurement
      *     @param measurementName - name of measurment
      *     @param mValue - value
      *     @param mUnit - unit (V, mA, etc.)
      *     @return 1 in case of success
      *     @return -2 error reading response (impossible to determine success)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int sendMeasurement(const char* type, const char* time, const char* fragmentName, const char* measurementName, const long &mValue, const char* mUnit);

    /**
      * Sends measurement to the server. Makes additional request to obtain current time.
      *     @param type - type of measurement
      *     @param measurementName - name of measurment
      *     @param mValue - value
      *     @param mUnit - unit (V, mA, etc.)
      *     @return 1 in case of success
      *     @return -2 error reading response (impossible to determine success)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int sendMeasurement(const char* type, const char* fragmentName, const char* measurementName, const long &mValue, const char* mUnit);
    int sendMeasurement(const char* type, const char* fragmentName, const char* measurementName, char* measurementString, const char* mUnit);
    int sendMeasurement(const char* type, const char* time, const char* fragmentName, const char* measurementName, char* measurementString, const char* mUnit);

    /**
      * Registers the arduino device in Cumulocity (using "Arduino" as type).
      * If already registered, skips registration and only copies id.
      *     @param name - name of the device
      *     @param[out] buffer - buffer for ID of the device
      *     @param bufferSize - size of the buffer
      *     @param supportedOperations - supported operations
      *     @param nSupportedOperations - size of supported operations
      *     @param supportedMeasurements - supported measurements
      *     @param nSupportedMeasurements - size of supported measurements
      *     @return 1 in case of success
      *     @return 0 in case device already existed on server
      *     @return -2 error reading response (impossible to determine success)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int registerDevice(const char* name, char* buffer, const int bufferSize,
        const char** supportedOperations, const int nSupportedOperations,
        const char** supportedMeasurements, const int nSupportedMeasurements);

	/**
	  * Registers device with no supported operations
	  *
	  */
    int registerDevice(const char* name, char* buffer, const int bufferSize,
        const char** supportedMeasurements, const int nSupportedMeasurements);

    /**
      * Registers the arduino device in Cumulocity (using "Arduino" as type).
      * If already registered, skips registration and only copies id.
      *     @param name - name of the device
      *     @param[out] buffer - buffer for ID of the device
      *     @param bufferSize - size of the buffer
      *     @return 1 in case of success
      *     @return 0 in case device already existed on server
      *     @return -2 error reading response (impossible to determine success)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int registerDevice(const char* name, char* buffer, const int bufferSize);

	/**
	  * Links external ID to registered device
	  */
	int linkID(char* intId, char* extId);
    /**
      * Obtains oldest pending operation from server (if any). If the operation fragment is a string,
      * it will be copied copied to buffer, excluding quotes. If the operation fragment is an object
      * it will be copied to buffer including brackets.
      *     @param[out] operationName - buffer for name of the operation
      *     @param nameBuffLength - length of the operationName buffer
      *     @param[out] operationBuffer - buffer for value of the operation
      *     @param opBuffLength - length of the operationBuffer buffer
      *     @return 1 in case of success
      *     @return 0 in case of no pending operations
      *     @return -2 error reading response (could not read operation)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int getPendingOperationFromServer(char* operationName, int nameBuffLength, char* operationBuffer, int opBuffLength);

    /**
      * Marks the oldest available pending operation as completed.
      *     @return 1 in case of success
      *     @return 0 in case of no pending operations to update
      *     @return -2 error reading response (could not read operation)
      *     @return -3 in case of timeout reading response
      *     @return other negative value as http error multiplied by -1
      */
    int markOperationCompleted();

    /**
      * If required interval passed and there is a pending operation on server,
      * handler for this command will be invoked.
      * @see registerForServerOperation
      */
    void triggerHandlersIfCommandsAwaiting();

    /**
      * Register a handler for server operation. Requires triggerHandlersIfCommandsAwaiting method to be called
      * in the loop method
      *     @param handler - handler for operation
      *     @param operationFragmentName - name of the fragment containing operation
      *     @param operationBuffer - buffer for operation
      *     @param bufferLength - length of the buffer
      *     @param interval - how often should server be checked for new commands
      * @see triggerHandlersIfCommandsAwaiting
      * @see getPendingOperationFromServer
      */
    void registerForServerOperation(void (*functionPtr)(), char* operationNameBuffer, int nameBufferLength, char* operationBuffer, int operationBufferLength, const unsigned long interval);


    /**
      * Copies pointer to time string in format yy-MM-ddThh:mm:ss.SSSZ. The pointer will always point to the same
      * char array, stored in this object. Method getTime will update this char array, returning it in the end.
      *     @return pointer to char array containing time
      */
    const char* getTime();

	/**
	  * Write Telstra IoT credentials to secure non-volatile memory on IoT Device
	  */
	void writeCredentials(const char *deviceID,const char *deviceTenant,const char *deviceUsername,const char *devicePassword);

	/**
	  * Read Telstra IoT credentials from secure non-volatile memory on IoT Device
	  */
	void readCredentials(char *deviceID,char *deviceTenant,char *deviceUsername,char *devicePassword);

	/**
	  * Clear Telstra IoT credentials from secure non-volatile memory on IoT Device
	  */
	void clearCredentials();
	void setCredentials(const char* _deviceId, const char* _tenantId, const char* _user, const char* _password, const char* _applicationKey);
	void setHost(const char* _host, const int _port);
};

/* -------------------- BASE64 -------------------- */

static const char* base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
void base64_encode(char* output, char const* input, unsigned int in_len);

#endif /* TelstraIoT_H_ */
