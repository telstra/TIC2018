#include <connection4g.h>
#include <telstraiot.h>
#include <TelstraM1Device.h>
#include <TelstraM1Interface.h>
#include <reporting.h>

#define BUTTON 7
#define LED 13

TelstraM1Interface commsif;
TelstraM1Device IoTDevice(&commsif);
Connection4G conn(true,&commsif);

TelstraIoT iotPlatform(&conn,&IoTDevice);

const char host[] = "tekt.iotdev.telstra.com";

char id[8];
char tenant[32];
char username[32];
char password[32];
bool led_state=HIGH;
unsigned int requestNumber = 0;
void setup() {
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); 
  commsif.begin(); // Must include for proper SPI communication
  rep_setLevel(REPORT_L2);
  Serial.begin(115200);
  //while(!Serial){}
  delay(1000);  

  reportln(F("[START] Starting Send All Measurments Script"));
  
  if(!IoTDevice.isCellularSystemReady())
  {
    
    reportln("waiting for IoTDevice ...");
    
    IoTDevice.waitUntilCellularSystemIsReady();
  } else {
    
    reportln("IoTDevice ready!");
    
  }

  //while(Serial==0); // Wait for serial character before starting
  
  reportln("Reading credentials...");
  
  IoTDevice.readCredentials(id,tenant,username,password);
  
    report("ID: ");
    reportln(id);
    report("Tenant: ");
    reportln(tenant); 
    report("Username: ");
    reportln(username);
    report("Password: ");
    reportln(password);
    
    reportln("Setting credentials...");
  
    iotPlatform.setCredentials(id,tenant,username,password,"");
  
    reportln("Setting host...");
  
  iotPlatform.setHost(host,443);
  conn.openTCP(host,443);  

}

void loop() {
   int resp=0;
    led_state=!led_state;
    digitalWrite(LED,led_state);
    delay(5000);
    char lightString[15];
    //char tempString[15];
    
    reportln("############################ Preparing to read MEASUREMENTS #############################");
    
    //Read Light measurement from device
    if(resp=IoTDevice.getLightLevel(lightString)==TELSTRAIOT_STATUS_OK)
    {
      report(F("[    ] Light: "));
      reportln(lightString);
      
      
      reportln("############################ Preparing to send MEASUREMENTS #############################");  
      
    
      resp = iotPlatform.sendMeasurement("LightMeasurement", "LightMeasurement", "Light level (lux)", lightString, "lux");
      delay(100);
      
      report("Command response code = ");
      reportln(resp);
    }
    
}

