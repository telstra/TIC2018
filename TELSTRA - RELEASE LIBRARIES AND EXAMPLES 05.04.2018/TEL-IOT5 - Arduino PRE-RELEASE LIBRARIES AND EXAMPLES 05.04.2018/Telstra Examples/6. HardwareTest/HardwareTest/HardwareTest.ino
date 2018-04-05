#include <connection4g.h>
#include <telstraiot.h>
#include <TelstraM1Device.h>
#include <TelstraM1Interface.h>
#include <gps.h>

#define BUTTON 7
#define LED 13
TelstraM1Interface commsif;
TelstraM1Device IoTDevice(&commsif);
Connection4G conn(true,&commsif);
GPS gps(&commsif);
GPS_Location * location;


void setup() {
  //delay(2000);

  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); 
  commsif.begin(); // Must include for proper SPI communication
	Serial.begin(115200);
}

void loop() {
  while(Serial.available() || (digitalRead(BUTTON)));
  serialFlush();
  digitalWrite(LED, LOW); 

  Serial.println("Waiting until cellular system has finished booting...");
  IoTDevice.waitUntilCellularSystemIsReady();

  delay(1000);
  Serial.println("Getting BG96 Dev Kit status...");
  GetInfo();
  
  Serial.println("Checking sensors...");
  char temperatureString[15];
  IoTDevice.getTemperature(temperatureString);
  Serial.print("  Read temperature: ");
  Serial.println(temperatureString);
  
  char lightString[15];
  IoTDevice.getLightLevel(lightString);
  Serial.print("  Read light level: ");
  Serial.println(lightString);

  Serial.println("Checking battery...");
  int batteryState = IoTDevice.getBatteryStatus();
  Serial.print("  Status: ");
  Serial.println(batteryState);
  int batterySoC = IoTDevice.getBatteryStateOfCharge();
  Serial.print("  State of Charge: ");
  Serial.println(batterySoC);

  gps.getStatus();
  Serial.print("Enabling GPS... ");
  if (gps.enable()) {
    Serial.println("OK!");
    Serial.println("GPS Locking might take several minutes depending on your location.");
    Serial.print("(Press any key to skip)\r\nWaiting... ");
    location = NULL;
    
    while(location == NULL) {
      if (Serial.available() || (digitalRead(BUTTON) == 0)) {
        serialFlush();
        break;  
      }
      location = gps.get_Location();
      delay(500);
    }
    
    if (location != NULL) {
      digitalWrite(LED, HIGH); 
      Serial.println("Locked!");
      Serial.print("Latitude :");   Serial.println(location->latitude);
      Serial.print("Longitude :");  Serial.println(location->longitude);
      Serial.print("Altitude: ");   Serial.println(location->altitude);
      Serial.print("Precision: ");   Serial.println(location->precision);
      Serial.print("Positioning Mode (2D/3D): ");   Serial.println(location->positioning_mode);
      Serial.print("Course of Ground: ");   Serial.println(location->cog);
      Serial.print("Speed [kmh]: ");   Serial.println(location->speed_kmh);
      Serial.print("Speed [knot]: ");   Serial.println(location->speed_knot);
      Serial.print("Number of satelites: ");   Serial.println(location->nsat);
  
      Serial.print("Disabling GPS... ");
      if (gps.disable()) 
        Serial.println("OK!");
      else Serial.println("Failed!");
    } else {
      Serial.println("Skipped");
    }
  } else Serial.println("Failed!");

  Serial.println("Updating RTC using network time...");
  char timebuffer[50];
  IoTDevice.updateRTCFromNetwork(timebuffer);
  Serial.print("  Updated time: ");
  Serial.println(timebuffer);

  Serial.println("Getting 4G network signal quality...");
  char sigbuffer[50];
  conn.getSignalQuality(sigbuffer);
  Serial.print("  Signal level: ");
  Serial.print(sigbuffer);
  Serial.println("dBm");

  Serial.println("Pinging www.google.com...");
  char ip[100];
  char hostname[] = "www.google.com";
  int responseTime = conn.ping(hostname, ip);

  Serial.print("  Host: ");
  Serial.print(hostname);
  Serial.print(", resolved IP address: ");
  Serial.print(ip);
  Serial.print(", response time: ");
  Serial.print(responseTime);
  Serial.println("ms");
  
  char hostname2[] = "ctofarm.iot.telstra.com";
  Serial.println("Opening secure TCP connection to ctofarm.iot.telstra.com...");
  if(conn.openTCP(hostname2,443)==CONNECTION4G_STATUS_OK)
  {
     Serial.println(" Success!");
     Serial.println(" Disconnecting...");
     conn.closeTCP();
  } else {
     Serial.println(" OpenTCP() failed.");
  }
  Serial.println("Done!");

}

void GetInfo()
{  
  char tm[200];
  IoTDevice.getIMEI(tm);
  Serial.print("  IMEI: ");
  Serial.println(tm);

  IoTDevice.getIP(tm);
  Serial.print("  IP: ");
  Serial.println(tm);
  
  IoTDevice.getTime(tm);
  Serial.print("  Time: ");
  Serial.println(tm);

  if(IoTDevice.isBG96Connected())
  {
    Serial.println("  BG96 connected!");  
  } else {
    Serial.println("  BG96 not connected.");      
  }
  
  if(IoTDevice.isSIMReady())
  {
    Serial.println("  SIM ready!");  
  } else {
    Serial.println("  SIM not ready.");      
  }
  
  if(IoTDevice.isPDPContextActive())
  {
    Serial.println("  PDP active!");  
  } else {
    Serial.println("  PDP not active.");      
  }
}

void serialFlush(){
  while(Serial.available() > 0) {
    Serial.read();
  }
}
