#include <connection4g.h>
#include <telstraiot.h>
#include <TelstraM1Device.h>
#include <TelstraM1Interface.h>

#define BUTTON 7
#define LED 13

TelstraM1Interface commsif;
TelstraM1Device IoTDevice(&commsif);
Connection4G conn(true,&commsif);

#define DeviceName "XXXX"

const char host[] = "YYYY.iotdev.telstra.com";
const char tenant[] = "YYYY";
const char username[] = "ZZZZ";
const char password[] = "XYXYXYXYX";
const char appkey[] = ""; 


TelstraIoT iotPlatform(host, tenant, username, password, appkey, &conn, &IoTDevice);

void setup() {
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); 
  commsif.begin(); // Must include for proper SPI communication
  Serial.begin(115200);
  delay(5000);  
  
  Serial.print(F("[START] Starting Registration Script using device name: "));
  Serial.println(DeviceName);

  while(!Serial); // Wait for serial character before starting

  Serial.println(F("[    ] ******* Waiting for Cellular Communication *********"));
  IoTDevice.waitUntilCellularSystemIsReady();
  Serial.println(F("[    ] ******* Cellular Communication ready *********"));
  
  
  // Check if IoTDevice is connected to 4G network
  if(IoTDevice.isPDPContextActive()) {
        Serial.println(F("[ OK ] ******* Connected to network *********"));
        
    	  // Open secure TCP connection to IoT Platform host
        Serial.println("############################ OPENING TCP CONNECTION #########################");
        conn.openTCP(host,443);  
    
     	  // Register device on IoT Platform
        Serial.println("############################ REGISTERING DEVICE #############################");
        char id[8];
        const char* supportedMeasurements[1];
        supportedMeasurements[0] = "LightMeasurement";
        //supportedMeasurements[0] = "Temperature";
        int result = iotPlatform.registerDevice(DeviceName, id, 8, supportedMeasurements, 1);
    
        if(result<0) {
          Serial.println(F("[FAIL] Registration error. Please retry."));
          while(true);
        } else {
    	    Serial.print(F("[    ] Arduino registered with id: "));
          Serial.println(id);
    	  }
      
      	// Close the TCP connection to the server
        Serial.println("############################ REGISTRATION COMPLETE #########################");
        Serial.println("Please wait while connection closes...");
      	conn.closeTCP();
        Serial.println("Done!");
        delay(1000);


        
	
  } else {
        Serial.println(F("[FAIL] IoTDevice is not connected to 4G network!"));
        while(true);
        {
          delay(5000);
          Serial.println(F("[FAIL] IoTDevice is not connected to 4G network!"));
        }
  }
}

void loop() {

}

