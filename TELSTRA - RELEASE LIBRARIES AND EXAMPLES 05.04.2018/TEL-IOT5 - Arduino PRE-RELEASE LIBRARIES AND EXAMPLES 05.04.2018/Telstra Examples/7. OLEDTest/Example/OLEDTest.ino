
#include "OLED.h"
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
OLED OLED;

//============================================================================

//!!-----------------------OLED---------------------------------------------!!
void setup(){
  //------------------------------Telstra Comms SETUP----------------------
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH); 
  commsif.begin(); // Must include for proper SPI communication
  
  //debug console
  Serial.begin(115200);
  Serial.println("setup()");
  //------------------------------Telstra Comms SETUP----------------------

  //------------------------------OLED SETUP----------------------

  randomSeed(123); // Used for randomising colours
  
  //Initialize the OLED controller
  OLED.begin();
  OLED.fill_OLED(0x00,0x00,0x00); // Clear screen
  //!!------------------------------OLED SETUP----------------------!!
  delay(10);
  //while(Serial.available() || (digitalRead(BUTTON)));
  serialFlush();

  OLED.fill_OLED(0xFF,0xFF,0xFF); 
  OLED.drawImage(logo, 50, 3, 28, 31); // Draw Teltra logo to screen

  delay(5000);
  
  Serial.println("Waiting until cellular system has finished booting...");
  while(!IoTDevice.isCellularSystemReady())
  { 
    OLED.fill_OLED(0x00,0x00,0x00);
    OLED.drawString("Connecting", 15, 15, 255, 255, 255,1);
    delay(300);
    OLED.drawString("Connecting .", 15, 15, 255, 255, 255,1); 
    delay(300);
    OLED.drawString("Connecting . .", 15, 15, 255, 255, 255,1); 
    delay(300);
    OLED.drawString("Connecting . . .", 15, 15, 255, 255, 255,1); 
    delay(300);
  }

  while(!IoTDevice.isPDPContextActive())
  { 
    OLED.fill_OLED(0x00,0x00,0x00);
    OLED.drawString("Connecting", 15, 15, 255, 255, 255,1);
    delay(300);
    OLED.drawString("Connecting .", 15, 15, 255, 255, 255,1); 
    delay(300);
    OLED.drawString("Connecting . .", 15, 15, 255, 255, 255,1); 
    delay(300);
    OLED.drawString("Connecting . . .", 15, 15, 255, 255, 255,1); 
    delay(300);
  }

  Serial.println("Getting BG96 Dev Kit status...");
  GetInfo();
  
  while(digitalRead(BUTTON));
  
  Serial.println("Running OLED Code");
}
//============================================================================
void loop()
  {
  uint8_t
    i;
  uint16_t
    x;
  uint16_t
    sub_x;
  uint16_t
    y;
  uint16_t
    sub_y;
  uint8_t
    r;
  uint8_t
    g;
  uint8_t
    b;
/*
  //Fill Demo
  Serial.println("Fill Demo");
  for(i=0;i<2;i++)
  {
    OLED.fill_OLED(0x00,0x00,0x00);
    delay(250);
    OLED.fill_OLED(0x00,0x00,0xFF);
    delay(250);
    OLED.fill_OLED(0x00,0xFF,0x00);
    delay(250);
    OLED.fill_OLED(0x00,0xFF,0xFF);
    delay(250);
    OLED.fill_OLED(0xFF,0x00,0x00);
    delay(250);
    OLED.fill_OLED(0xFF,0x00,0xFF);
    delay(250);
    OLED.fill_OLED(0xFF,0xFF,0x00);
    delay(250);
    OLED.fill_OLED(0xFF,0xFF,0xFF);
    delay(250);
  }
  Serial.println("Cheesy lines demo");
  //Cheesy lines demo
  r=0xff;
  g=0x00;
  b=0x80;
  for(i=0;i<10;i++)
  {
    for(x=0;x<128;x++)
    {
      OLED.drawLine(63,17,
               x,0,
               r++,g--,b+=2);
    }
    for(y=0;y<36;y++)
    {
      OLED.drawLine(63,17,
               127,y,
               r++,g+=4,b+=2);
    }
    for(x=127;0!=x;x--)
    {
      OLED.drawLine(63,17,
               x,35,
               r-=3,g-=2,b-=1);
    }
    for(y=35;0!=y;y--)
    {
      OLED.drawLine(63,17,
               0,y,
               r+-3,g--,b++);
    }
  }

  //Circle demo
  Serial.println("Circle demo");
  OLED.fill_OLED(0x00,0x00,0x00);
  r=0xff;
  g=0x00;
  b=0x80;
  for(i = 0; i < 20; i++)
  {
    for(x = 2; x < 114; x+=2)
    {
      OLED.drawCircle(x+2, 17, 2+x%15,r+-6,g-=4,b+=11);
    }
  }

  //8x8 checkerboard demo
  Serial.println("8x8 checkerboard demo");
  OLED.fill_OLED(0x00,0x00,0x00);
  for(i=0;i<20;i++)
  {
    //Write a 8x8 checkerboard
    for(x=0;x<=15;x++)
    {
      for(y=0;y<=5;y++)
      {
        for(sub_x=0;sub_x<=7;sub_x++)
        {
          for(sub_y=0;sub_y<=7;sub_y++)
          {
            if(((x&0x01)&&!(y&0x01)) || (!(x&0x01)&&(y&0x01)))
            {
              OLED.drawPixel((x<<3)+sub_x,(y<<3)+sub_y, 0x00, 0x00, 0x00);
            }
            else
            {
              OLED.drawPixel((x<<3)+sub_x,(y<<3)+sub_y,0xFF-((x+i)<<4), 0xFF-((x<<5)+(i<<3)), 0xFF-(y<<6));
            }
          }
        }
      }
    }
  }
*/

  Serial.println("Text demo");
  OLED.fill_OLED(0x00,0x00,0x00);
  
  r = random(0,255);
  g = random(0,255);
  b = random(0,255);

  OLED.drawChar('a',0,10,r,g,b,1);
  OLED.drawChar('b',6,10,r,g,b,1);
  OLED.drawChar('c',12,10,r,g,b,1);
  OLED.drawChar('1',18,10,r,g,b,1);
  OLED.drawChar('2',24,10,r,g,b,1);
  OLED.drawChar('3',30,10,r,g,b,1);

  OLED.drawChar('a',0,19,r,g,b,2);
  OLED.drawChar('b',11,19,r,g,b,2);
  OLED.drawChar('c',22,19,r,g,b,2);
  OLED.drawChar('1',33,19,r,g,b,2);
  OLED.drawChar('2',44,19,r,g,b,2);
  OLED.drawChar('3',55,19,r,g,b,2);
  
  for (x = 0; x < 128; x++) 
  {
    for (i = 0; i < 5; i++) 
    {
      OLED.drawLine(0,i,
               x,i,
               r,g,b);
    }
    delay(6);
  }

  r = random(0,255);
  g = random(0,255);
  b = random(0,255);

  OLED.drawChar('A',0,10,r,g,b,1);
  OLED.drawChar('B',6,10,r,g,b,1);
  OLED.drawChar('C',12,10,r,g,b,1);
  OLED.drawChar('1',18,10,r,g,b,1);
  OLED.drawChar('2',24,10,r,g,b,1);
  OLED.drawChar('3',30,10,r,g,b,1);

  OLED.drawChar('A',0,19,r,g,b,2);
  OLED.drawChar('B',11,19,r,g,b,2);
  OLED.drawChar('C',22,19,r,g,b,2);
  OLED.drawChar('1',33,19,r,g,b,2);
  OLED.drawChar('2',44,19,r,g,b,2);
  OLED.drawChar('3',55,19,r,g,b,2);

  for (x = 127; x > 0; x--) 
  {
    for (i = 0; i < 5; i++) 
    {
      OLED.drawLine(127,i,
               x,i,
               r,g,b);
    }
    delay(6);
  }
  OLED.fill_OLED(0x00,0x00,0x00);
  OLED.drawString("Hello World!",0,15,r,g,b,1);
  delay(1000);
} // void loop()
//============================================================================

void GetInfo()
{  
  char tm[200];
  IoTDevice.getIMEI(tm);
  Serial.print("  IMEI: ");
  Serial.println(tm);

  OLED.fill_OLED(0x00,0x00,0x00);

  String str = "IMEI: " + String(tm);;
  OLED.drawString(str,0,10,255,255,255,1);

  IoTDevice.getIP(tm);
  Serial.print("  IP: ");
  Serial.println(tm);

  str = "IP: " + String(tm);
  OLED.drawString(str,0,22,255,255,255,1);
  
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

