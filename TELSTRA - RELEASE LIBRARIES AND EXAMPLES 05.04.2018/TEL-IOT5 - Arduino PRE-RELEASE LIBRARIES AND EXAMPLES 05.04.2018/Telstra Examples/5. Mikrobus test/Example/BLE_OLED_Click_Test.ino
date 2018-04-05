

// Test script for MikroBus OLED screen and BLE UART link

// NOTE:
// Requires Header H2, Pin 10 (P0.14 on the BMD-350) to be shorted to ground during startup.
// This enables AT command mode in the RigaBlue firmware.
// otherwise the program will hang waiting for BLE response

#include "OLED.h"
#include <connection4g.h>
#include <telstraiot.h>
#include <TelstraM1Device.h>
#include <TelstraM1Interface.h>
#include <gps.h>

#define LED_MIKROBUS_RESET 13 // mikrobus reset line and LED pin

TelstraM1Interface commsif;
TelstraM1Device IoTDevice(&commsif);
Connection4G conn(true,&commsif);
GPS gps(&commsif);
GPS_Location * location;
OLED OLED;

//============================================================================
void setup(){

  pinMode(LED_MIKROBUS_RESET, OUTPUT);
  
  //debug console
  Serial.begin(57600);
  Serial1.begin(57600);
  Serial.println("setup()");

  commsif.begin(); // Must include for proper operation
  
  //Initialize the OLED controller
  OLED.begin();
  OLED.fill_OLED(0x00,0x00,0x00); // Clear screen

  delay(10);
  serialFlush();

  OLED.fill_OLED(0xFF,0xFF,0xFF); 
  OLED.drawImage(logo, 50, 3, 28, 31); // Draw Teltra logo to screen

  delay(1000);

  OLED.fill_OLED(0x00,0x00,0x00);
  OLED.drawString("WAIT for BLE response", 1, 15, 255, 255, 255,1);
  
  Serial.println("Running Test Code");
  delay(100);
}
//============================================================================
void loop()
{
  Serial.println("Testing BLE module UART");
  serial_1_Flush();
  Serial1.write("AT\n");
  int rx_count = 0;
  char rx_str[5];
  while(rx_count < 2){
    if(Serial1.available()){
      rx_str[rx_count] = Serial1.read();
      rx_count++;
    }
  }
  
  rx_str[rx_count] = '\0';
  Serial.print("Received :");
  Serial.println(rx_str);
  if(memcmp(rx_str,"OK",2) == 0){
      OLED.fill_OLED(0x00,0x00,0x00);
      OLED.drawString("PASS", 15, 15, 255, 255, 255,1);
      Serial.println("PASS");
  }else{
      OLED.fill_OLED(0x00,0x00,0x00);
      OLED.drawString("FAIL", 15, 15, 255, 255, 255,1);  
      Serial.println("FAIL");
  }

  delay(1000);
} 
//============================================================================

void serialFlush(){
  while(Serial.available() > 0) {
    Serial.read();
  }
}

void serial_1_Flush(){
  while(Serial1.available() > 0) {
    Serial1.read();
  }
}
