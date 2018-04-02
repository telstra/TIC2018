#include <connection4g.h>
#include <telstraiot.h>
#include <TelstraM1Device.h>
#include <TelstraM1Interface.h>

#define BUTTON 7
#define LED 13

TelstraM1Interface commsif;
TelstraM1Device IoTDevice(&commsif);
Connection4G conn(true,&commsif);

TelstraIoT iotPlatform(&conn,&IoTDevice);

const char host[] = "YYYY.iotdev.telstra.com";

char id[8];
char tenant[32];
char username[32];
char password[32];

void setup() {
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); 
  commsif.begin(); // Must include for proper SPI communication
  Serial.begin(115200);
  delay(5000);  

  Serial.println(F("[START] Starting Send All Measurments Script"));
  
  if(!IoTDevice.isCellularSystemReady())
  {
     Serial.println("waiting for IoTDevice ...");
    IoTDevice.waitUntilCellularSystemIsReady();
  } else {
    Serial.println("IoTDevice ready!");
  }
  while(Serial.available()==0); // Wait for serial character before starting

  Serial.println("Reading credentials...");
  IoTDevice.readCredentials(id,tenant,username,password);
  Serial.print("ID: ");
  Serial.println(id);
  Serial.print("Tenant: ");
  Serial.println(tenant); 
  Serial.print("Username: ");
  Serial.println(username);
  Serial.print("Password: ");
  Serial.println(password);
  
  Serial.println("Setting credentials...");
  iotPlatform.setCredentials(id,tenant,username,password,"");
  
  Serial.println("Setting host...");
  iotPlatform.setHost(host,443);
  
  conn.openTCP(host,443);  

}

void loop() {
    delay(1000);
    char lightString[15];
    //char tempString[15];
    
    Serial.println("############################ Preparing to read MEASUREMENTS #############################");
    //Read Light measurement from device
    IoTDevice.getLightLevel(lightString);
    Serial.print(F("[    ] Light: "));
    Serial.println(lightString);

    /*
    //Read temperature measurement from device
    IoTDevice.getTemperature(tempString);
    Serial.print(F("[    ] Temp: "));
    Serial.println(tempString);
    */

    Serial.println("############################ Preparing to send MEASUREMENTS #############################");  
    iotPlatform.sendMeasurement("LightMeasurement", "LightMeasurement", "Light level (lux)", lightString, "lux");
    /*
    IoTDevice.getTemperature(tempString);
    iotPlatform.sendMeasurement("TemperatureMeasurement", "TemperatureMeasurement", "Temperature (degrees Celsius)", tempString, "degrees Celsius");
    */
}

