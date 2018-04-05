#include <TelstraWeb.h>
#include <connection4g.h>
#include <TelstraM1Device.h>
#include <TelstraM1Interface.h>
#include <reporting.h>

#define SECURE_CONNECTION true
#define INSECURE_CONNECTION false
#define POSTEXAMPLE "User-Agent: Arduino\r\nContent-Type: application/json\r\nX-IS-AccessKey: v0NmtMreZ7ahD8db7OjOhObbQCztaEEP\r\nX-IS-BucketKey: ARKC7VQRGNSV\r\nAccept-Version: ~0\r\nContent-Length: 30\r\n\r\n{\"key\":\"pickle\",\"value\":\"3.0\"}"

#define BUTTON 7
#define LED 13

//--Secure---
//char host[] = "";
//char path[] = "";
//int port = 443;
//--Secure---!

//--Unsecure---
char host[] = "httpbin.org";
char path[] = "/ip";
int port = 80;
//--Unsecure---

TelstraM1Interface commsif;
TelstraM1Device IoTDevice(&commsif);
Connection4G conn(INSECURE_CONNECTION,&commsif);
TelstraWeb WebIoT(&conn,&IoTDevice);

void setup() {
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); 
  
  commsif.begin(); // Must include for proper SPI communication
  Serial.begin(115200);
  rep_setLevel(REPORT_L2);
  Serial.println("Waiting until Cellular System is ready...");
    if(!IoTDevice.isCellularSystemReady())
  {
    
    Serial.println("waiting for IoTDevice ...");
    
    IoTDevice.waitUntilCellularSystemIsReady();
  } else {
    
    Serial.println("IoTDevice ready!");
    
  }

  delay(1000);
  Serial.println("Getting Dev Kit status...");
  GetInfo();

  WebIoT.setHost(host,path,port);
}

void loop() {
  while(Serial.available() || (digitalRead(BUTTON)));
  Serial.println(" Opening TCP connection!");
 if(conn.openTCP(host,port)==CONNECTION4G_STATUS_OK)
  {
    Serial.println(" Success!");
  
    delay(1000);
    // Build HTTPS request.
    WebIoT.post(POSTEXAMPLE);
    delay(2000);
    conn.closeTCP();

    } else {
     Serial.println(" OpenTCP() failed.");
  }
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
