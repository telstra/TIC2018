#include <connection4g.h>
#include <telstraiot.h>
#include <iotshield.h>
#include <shieldinterface.h>
#include "CurieIMU.h"
#include <stdio.h>

ShieldInterface shieldif;
IoTShield shield(&shieldif);
Connection4G conn(true,&shieldif);

TelstraIoT iotPlatform(&conn,&shield);

const char host[] = "tic2017team000.iot.telstra.com";

char id[8];
char tenant[32];
char username[32];
char password[32];
char ax_string[10];
char ay_string[10];
char az_string[10];

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

   CurieIMU.begin();
   CurieIMU.setAccelerometerRange(2);
}

void loop() {
    delay(2000);

    float ax, ay, az; //scaled accelerometer values

    Serial.println(F("############################ Preparing to send  MEASUREMENT #############################"));

    // read accelerometer measurements from device, scaled to the configured range
    CurieIMU.readAccelerometerScaled(ax, ay, az);
      // convert to m/s^2
    ax = ax*9.81;
    ay = ay*9.81;
    az = az*9.81;
    Serial.print(F("[    ] AccX: "));
    Serial.println(ax);
    Serial.print(F("[    ] AccY: "));
    Serial.println(ay);
    Serial.print(F("[    ] AccZ: "));
    Serial.println(az);

    /*
     * This converts the accelerometer floats to strings so they can be sent using
     * the sendMeasurement function
     */
    Serial.println("############################ SENDING X Acceleration #############################");
    dtostrf(ax, 4, 2, ax_string);
    iotPlatform.sendMeasurement("XAcc", "XAcc", "X Acceleration (m/s^2)", ax_string, "m/s^2");

    Serial.println("############################ SENDING Y Acceleration #############################");
    dtostrf(ay, 4, 2, ay_string);
    iotPlatform.sendMeasurement("YAcc", "YAcc", "Y Acceleration (m/s^2)", ay_string, "m/s^2");

    Serial.println("############################ SENDING Z Acceleration #############################");
    dtostrf(az, 4, 2, az_string);
    iotPlatform.sendMeasurement("ZAcc", "ZAcc", "Z Acceleration (m/s^2)", az_string, "m/s^2");

}

