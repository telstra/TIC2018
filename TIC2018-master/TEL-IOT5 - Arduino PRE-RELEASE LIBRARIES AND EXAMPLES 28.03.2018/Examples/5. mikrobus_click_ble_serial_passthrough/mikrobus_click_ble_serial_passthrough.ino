/*
  This sketch allows you to  emulate the serial passthrough behaviour.
  Any text you type in the IDE Serial monitor will be written
  out to the MikroBus serial port , and vice-versa.

  On the 101, MKR1000, Zero, and Micro, "Serial" refers to the USB Serial port
  attached to the Serial Monitor, and "Serial1" refers to the hardware
  serial port attached to pins 0 and 1 (And the MikroBus). This sketch will emulate Serial 
  passthrough using those two Serial ports on the boards mentioned above,
  but you can change these names to connect any two serial ports on a board
  that has multiple ports.

   Created 23 May 2016
   by Erik Nyquist 
   
   modified by Telstra
*/


/* 
 *  Telstra CAT-M1 Development Kit : Bluetooth LE serial emulation example
 *  
 *  Detailed documentation of the BMDware serial and Bluetooth API can be found at the following links
 *  BMDware Datasheet                             : http://www.rigado.com/?wpdmdl=1328
 *  Rigablue Datasheet – iOS & Android Libraries  : http://www.rigado.com/?wpdmdl=1330
 *  BMD-350 information                           : https://www.rigado.com/products/modules/bmd-350/
 *  
 *  
 *  Note: 
 *  Before the Bluetooth UART can be used, the OLED and BLE MikroBUS Click should be installed. 
 *  
 *  To enable BLE Serial emulation it must be enabled by writing a non-zero value to 
 *  the "UART Enable Characteristic" (UUID: 6E400008-B5A3-F393-E0A9-E50E24DCCA9E)
 *  This can be done by using the nRF Connect application on an Android and iOS phone.
 *  
 *  The baud rate should be set no higher than 57600, as the Bluetooth link can't transmit 
 *  faster than this rate and the Arduino serial link is not using hardware flow control.
 *  
 */


void setup() {
  Serial.begin(57600);
  Serial1.begin(57600);
}

void loop() {
  if (Serial.available()) {      // If anything comes in Serial (USB),
    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  }

  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
    Serial.write(Serial1.read());   // read it and send it out Serial (USB)
  }
}
