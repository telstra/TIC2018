#include <connection4g.h>
#include <telstraiot.h>
#include <iotshield.h>
#include <shieldinterface.h>

ShieldInterface shieldif;
IoTShield shield(&shieldif);
Connection4G conn(true,&shieldif);

TelstraIoT iotPlatform(&conn,&shield);

const char host[] = "tic2017team000.iot.telstra.com";

char id[8];
char tenant[32];
char username[32];
char password[32];

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("[START] Starting Send All Measurments Script"));
  
  if(!shield.isShieldReady())
  {
     Serial.println("waiting for shield ...");
    shield.waitUntilShieldIsReady();
  } else {
    Serial.println("Shield ready!");
  }
  //while(Serial.available()==0); // Wait for serial character before starting

  Serial.println("Reading credentials...");
  shield.readCredentials(id,tenant,username,password);
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
    shield.getLightLevel(lightString);
    Serial.print(F("[    ] Light: "));
    Serial.println(lightString);

    /*
    //Read temperature measurement from device
    shield.getTemperature(tempString);
    Serial.print(F("[    ] Temp: "));
    Serial.println(tempString);
    */

    Serial.println("############################ Preparing to send MEASUREMENTS #############################");  
    iotPlatform.sendMeasurement("LightMeasurement", "LightMeasurement", "Light level (lux)", lightString, "lux");
    /*
    shield.getTemperature(tempString);
    iotPlatform.sendMeasurement("TemperatureMeasurement", "TemperatureMeasurement", "Temperature (degrees Celsius)", tempString, "degrees Celsius");
    */
}

