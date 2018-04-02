
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

//-----------------------OLED---------------------------------------------
void Set_OLED_for_write_at_X_Y(uint8_t x, uint8_t y)
  {
  // Select the OLED controller
  CLR_CS;
  // Select the OLED's command register
  CLR_RS;
  //Send the command via SPI:
  SPI.transfer(LD7138_0x0A_SET_DATA_WINDOW);
    // Select the OLED's data register
    SET_RS;
    //Xstart, bits 6,5,4 right justified
    SPI.transfer((x>>4)&0x07);
    //Xstart, bits 3,2,1,0 right justified
    SPI.transfer(x&0x0F);
    #define XEND (127)
    //Xend, bits 6,5,4 right justified
    SPI.transfer((XEND>>4)&0x07);
    //Xend, bits 3,2,1,0 right justified
    SPI.transfer(XEND&0x0F);
    y+=60;
    //Ystart, bits 6,5,4 right justified
    SPI.transfer((y>>4)&0x07);
    //Ystart, bits 3,2,1,0 right justified
    SPI.transfer(y&0x0F);
    #define YEND (95)
    //Yend, bits 6,5,4 right justified
    SPI.transfer((YEND>>4)&0x07);
    //Yend, bits 3,2,1,0 right justified
    SPI.transfer(YEND&0x0F);
    #undef XEND
    #undef YEND

  // Select the OLED's command register
  CLR_RS;
  //Send the command via SPI:
  SPI.transfer(LD7138_0x0C_DATA_WRITE_READ);
  // Data will be sent by calling function
  // Deselect the OLED controller
  SET_CS;
  }
//============================================================================
// ~19.15mS
void Fill_OLED(uint8_t R, uint8_t G, uint8_t B)
  {
  uint16_t
    i;
  Set_OLED_for_write_at_X_Y(0, 0);

  //Pre-calculate the two bytes for this color of pixel/
  uint8_t
    first_half;
  uint8_t
    second_half;
  //The display takes two bytes (565) RRRRR GGGGGG BBBBB
  //to show one pixel.
  first_half=(R&0xF8) | (G >> 5);
  second_half=(G<<5) | (B >> 3);

  // Select the OLED controller
  CLR_CS;
  // Select the OLED's data register
  SET_RS;

  //Fill display with a given RGB value
  for (i = 0; i < (128*36); i++)
    {
    SPI.transfer(first_half);
    SPI.transfer(second_half);
    }
  // Deselect the OLED controller
  SET_CS;
  }
//============================================================================
void Initialize_OLED(void)
  {
  //Reset the OLED controller by hardware
  CLR_RESET;
  delay(1);
  SET_RESET;
  delay(100);

  // Software Reset(0x01h)
  // All registers are initialized with default value without altering
  // the graphic RAM.
  // All Dot display are turned OFF.
  // The OSC is stopped.
  SPI_sendCommand(LD7138_0x01_SOFTRES);
  // Set Dot Matrix Display ON/OFF (0x02h)
  //   P0 = 0: indicates the dot matrix display turns OFF(Default).
  //   P0 = 1: indicates the dot matrix Display turns ON.
  // NOTE: Display OFF means:
  //   * All Column Output go to pre-charge level.
  //   * All Row Output go to the ground level
  //   * Stop Data transfer from memory to Dot Matrix Driver.
  SPI_sendCommand(LD7138_0x02_DISPLAY_ON_OFF);
    SPI_sendData(0x00);   //0 = Off, 1 = On
  // Set Dot Matrix Display Stand-by ON/OFF(0x03h)
  //   P0=0: Indicates the dot oscillator is starting. And it does not make
  //         the dot matrix display turn ON.
  //   P0=1: Indicates the dot oscillator is stopping. And it make the dot
  //         matrix display OFF.
  // NOTE: After software or hardware reset command is executed, it makes
  //   dot matrix display stand-by ON.
  SPI_sendCommand(LD7138_0x03_DISPLAY_STANDBY_ON_OFF);
    SPI_sendData(0x00);  //0 = oscillator running
                         //1 = oscillator stopped/standby
    // Set OSC Control (0x04h)
  SPI_sendCommand(LD7138_0x04_OSCILLATOR_SPEED);
    SPI_sendData(0x02); // 90Hz
    // ---- MFFF
    //      ||||-- FFF:
    //      |       000 60Hz
    //      |       001 75Hz
    //      |       010 90Hz *
    //      |       011 105Hz
    //      |       100 120Hz
    //      |----- Mode:
    //              0 = Internal RC
    //              1 = External Clock
  // Set Graphic RAM Writing Direction (0x05h)
  SPI_sendCommand(LD7138_0x05_WRITE_DIRECTION);
    SPI_sendData(0x00);
    // ---- RDDD
    //      ||||-- DDD: See page 22 in datasheet
    //      |       Sets the up/down right left address
    //      |       increment order
    //      |----- Color Order:
    //              0 = RGB
    //              1 = BGR
  // Set Row Scan Direction (0x06h)
  SPI_sendCommand(LD7138_0x06_SCAN_DIRECTION);
    SPI_sendData(0x00);  //0 = min to max / increment
                         //1 = max to min / decrement

  // Set Display Size (0x07h)
  SPI_sendCommand(LD7138_0x07_SET_DISPLAY_WINDOW);
    #define XSTART (0)
    //Xstart, bits 6,5,4 right justified
    SPI_sendData((XSTART>>4)&0x07);
    //Xstart, bits 3,2,1,0 right justified
    SPI_sendData(XSTART&0x0F);
    #define XEND (127)
    //Xend, bits 6,5,4 right justified
    SPI_sendData((XEND>>4)&0x07);
    //Xend, bits 3,2,1,0 right justified
    SPI_sendData(XEND&0x0F);
    #define YSTART (60)
    //Ystart, bits 6,5,4 right justified
    SPI_sendData((YSTART>>4)&0x07);
    //Ystart, bits 3,2,1,0 right justified
    SPI_sendData(YSTART&0x0F);
    #define YEND (95)
    //Yend, bits 6,5,4 right justified
    SPI_sendData((YEND>>4)&0x07);
    //Yend, bits 3,2,1,0 right justified
    SPI_sendData(YEND&0x0F);
    #undef XSTART
    #undef XEND
    #undef YSTART
    #undef YEND

  // Set Interface Bus Type (0x08h)
  SPI_sendCommand(LD7138_0x08_IF_BUS_SEL);
    SPI_sendData(0x01);
    // ---- --II
    //        ||-- II:
    //               0 0 = 6 bit
    //               0 1 = 8 bit *
    //               1 1 = 16 bit (invalid for SPI/I2C)

  // Set Masking Data (0x09h)
  SPI_sendCommand(LD7138_0x09_DATA_MASKING);
    SPI_sendData(0x07);
    // ---I -RGB
    //    |  |||-- RGB: Show this data
    //    |------- Invert shown data

  // Set Read/WriteBoxData (0x0Ah)
  SPI_sendCommand(LD7138_0x0A_SET_DATA_WINDOW);
    #define XSTART (0)
    //Xstart, bits 6,5,4 right justified
    SPI_sendData((XSTART>>4)&0x07);
    //Xstart, bits 3,2,1,0 right justified
    SPI_sendData(XSTART&0x0F);
    #define XEND (127)
    //Xend, bits 6,5,4 right justified
    SPI_sendData((XEND>>4)&0x07);
    //Xend, bits 3,2,1,0 right justified
    SPI_sendData(XEND&0x0F);
    #define YSTART (60)
    //Ystart, bits 6,5,4 right justified
    SPI_sendData((YSTART>>4)&0x07);
    //Ystart, bits 3,2,1,0 right justified
    SPI_sendData(YSTART&0x0F);
    #define YEND (95)
    //Yend, bits 6,5,4 right justified
    SPI_sendData((YEND>>4)&0x07);
    //Yend, bits 3,2,1,0 right justified
    SPI_sendData(YEND&0x0F);
    #undef XSTART
    #undef XEND
    #undef YSTART
    #undef YEND
  // Set Display Start Address(0x0Bh)
  SPI_sendCommand(LD7138_0x0B_SET_ADDRESS);
    #define XSTART (0)
    //Xstart, bits 6,5,4 right justified
    SPI_sendData((XSTART>>4)&0x07);
    //Xstart, bits 3,2,1,0 right justified
    SPI_sendData(XSTART&0x0F);
    #define YSTART (0)
    //Ystart, bits 6,5,4 right justified
    SPI_sendData((YSTART>>4)&0x07);
    //Ystart, bits 3,2,1,0 right justified
    SPI_sendData(YSTART&0x0F);
    #undef XSTART
    #undef YSTART
  // Set Dot Matrix Current Level (0x0Eh)
  SPI_sendCommand(LD7138_0x0E_RGB_CURRENT_LEVEL);
    #define IRED (90)
    //IRED, bits 7,6,5,4 right justified
    SPI_sendData((IRED>>4)&0x0F);
    //IRED, bits 3,2,1,0 right justified
    SPI_sendData(IRED&0x0F);
    #define IGREEN (83)
    //IGREEN, bits 7,6,5,4 right justified
    SPI_sendData((IGREEN>>4)&0x0F);
    //IGREEN, bits 3,2,1,0 right justified
    SPI_sendData(IGREEN&0x0F);
    #define IBLUE (69)
    //IBLUE, bits 7,6,5,4 right justified
    SPI_sendData((IBLUE>>4)&0x0F);
    //IBLUE, bits 3,2,1,0 right justified
    SPI_sendData(IBLUE&0x0F);
  // Set Dot Matrix Peak Current Level (0x0Fh)
  SPI_sendCommand(LD7138_0x0F_PEAK_CURRENT_LEVEL);
    SPI_sendData(0x05); //Red
    SPI_sendData(0x10); //Green
    SPI_sendData(0x23); //Blue
  // Set Pre-Charge Width (0x1Ch)
  SPI_sendCommand(LD7138_0x1C_PRE_CHARGE_WIDTH);
    SPI_sendData(0x01);
  // Set Peak Pulse Width (0x1Dh)
  SPI_sendCommand(LD7138_0x1C_SET_PEAK_WIDTH);
    SPI_sendData(0x3F); //Red
    SPI_sendData(0x10); //Green
    SPI_sendData(0x3C); //Blue
  // Set Peak Pulse Delay (0x1Eh)
  SPI_sendCommand(LD7138_0x1E_SET_PEAK_DELAY);
    SPI_sendData(0x0F);
  //Set Row Scan Operation (0x1Fh)
  SPI_sendCommand(LD7138_0x1F_SET_ROW_SCAN);
    SPI_sendData(0x30);
    // ---TT E-II
    //    || | ||-- Interlace modes
    //    || |----- 0 = normal, 1 = Ground rows
    //    ||------- Precharge/peak/max flags
  // Set Internal Regulator for Row Scan (0x30h)
  SPI_sendCommand(LD7138_0x30_VCC_R_SELECT);
    SPI_sendData(0x11);
    // ----E -MMM
    //     |  |||-- Multiplier:
    //     |        000 = VCC_C * 0.85
    //     |        001 = VCC_C * 0.80 *
    //     |        010 = VCC_C * 0.75
    //     |        011 = VCC_C * 0.70
    //     |        100 = VCC_C * 0.65
    //     |------- 1 = Enable, 0 = Disable
  //Set VDD Selection(0x3Ch)
  //Controls voltage used for Iref feedback
  SPI_sendCommand(LD7138_0x3C_SET_VDD_SELECTION);
    SPI_sendData(0x00); // 0 = 2.8v, 1 = 1.8v

  Fill_OLED(0x00,0x00,0x00);

  // Set Dot Matrix Display ON/OFF (0x02h)
  //   P0 = 0: indicates the dot matrix display turns OFF(Default).
  //   P0 = 1: indicates the dot matrix Display turns ON.
  // NOTE: Display OFF means:
  //   * All Column Output go to pre-charge level.
  //   * All Row Output go to the ground level
  //   * Stop Data transfer from memory to Dot Matrix Driver.
  SPI_sendCommand(LD7138_0x02_DISPLAY_ON_OFF);
    SPI_sendData(0x01);   //0 = Off, 1 = On
  }
//============================================================================
#if(0) //simple, 30.38uS 8000
void Put_Pixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
  {
  Set_OLED_for_write_at_X_Y(x, y);
  //Write the single pixel's worth of data
  SPI_sendData((R&0xF8) | (G >> 5));
  SPI_sendData((G<<5) | (B >> 3));
  }
#else //faster: 27.75uS, bigger (70 bytes)
void Put_Pixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
  {
  // Select the OLED controller
  CLR_CS;
  // Select the OLED's command register
  CLR_RS;
  //Send the command via SPI:
  SPI.transfer(LD7138_0x0A_SET_DATA_WINDOW);
    // Select the OLED's data register
    SET_RS;
    //Xstart, bits 6,5,4 right justified
    SPI.transfer((x>>4)&0x07);
    //Xstart, bits 3,2,1,0 right justified
    SPI.transfer(x&0x0F);
    #define XEND (127)
    //Xend, bits 6,5,4 right justified
    SPI.transfer((XEND>>4)&0x07);
    //Xend, bits 3,2,1,0 right justified
    SPI.transfer(XEND&0x0F);
    y+=60;
    //Ystart, bits 6,5,4 right justified
    SPI.transfer((y>>4)&0x07);
    //Ystart, bits 3,2,1,0 right justified
    SPI.transfer(y&0x0F);
    #define YEND (95)
    //Yend, bits 6,5,4 right justified
    SPI.transfer((YEND>>4)&0x07);
    //Yend, bits 3,2,1,0 right justified
    SPI.transfer(YEND&0x0F);
    #undef XEND
    #undef YEND

  // Select the OLED's command register
  CLR_RS;
  //Send the command via SPI:
  SPI.transfer(LD7138_0x0C_DATA_WRITE_READ);
    // Select the OLED's data register
    SET_RS;
    SPI.transfer((R&0xF8) | (G >> 5));
    SPI.transfer((G<<5) | (B >> 3));

  // Deselect the OLED controller
  SET_CS;
  }
#endif
//============================================================================
// From: http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void OLED_Circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t R, uint16_t G, uint16_t B)
  {
  uint16_t x = radius;
  uint16_t y = 0;
  int16_t radiusError = 1 - (int16_t) x;

  while (x >= y)
    {
    //11 O'Clock
    Put_Pixel(x0 - y, y0 + x, R, G, B);
    //1 O'Clock
    Put_Pixel(x0 + y, y0 + x, R, G, B);
    //10 O'Clock
    Put_Pixel(x0 - x, y0 + y, R, G, B);
    //2 O'Clock
    Put_Pixel(x0 + x, y0 + y, R, G, B);
    //8 O'Clock
    Put_Pixel(x0 - x, y0 - y, R, G, B);
    //4 O'Clock
    Put_Pixel(x0 + x, y0 - y, R, G, B);
    //7 O'Clock
    Put_Pixel(x0 - y, y0 - x, R, G, B);
    //5 O'Clock
    Put_Pixel(x0 + y, y0 - x, R, G, B);

    y++;
    if (radiusError < 0)
      radiusError += (int16_t)(2 * y + 1);
    else
      {
      x--;
      radiusError += 2 * (((int16_t) y - (int16_t) x) + 1);
      }
    }
  }
//============================================================================
#define mSwap(a,b,t)\
  {\
  t=a;\
  a=b;\
  b=t;\
  }\
//----------------------------------------------------------------------------
void Fast_Horizontal_Line(uint16_t x0, uint16_t y, uint16_t x1,
                          uint8_t R, uint8_t G, uint8_t B)
  {
  uint16_t
    temp;
  if(x1 < x0)
    mSwap(x0, x1, temp);
  Set_OLED_for_write_at_X_Y(x0, y);

  //Pre-calculate the two bytes for this color of pixel/
  uint8_t
    first_half;
  uint8_t
    second_half;
  //The display takes two bytes (565) RRRRR GGGGGG BBBBB
  //to show one pixel.
  first_half=(R&0xF8) | (G >> 5);
  second_half=(G<<5) | (B >> 3);

  // Select the OLED controller
  CLR_CS;
  // Select the OLED's data register
  SET_RS;

  while(x0 <= x1)
    {
    //Write the single pixel's worth of data
    SPI.transfer(first_half);
    SPI.transfer(second_half);
    x0++;
    }
  // Deselect the OLED controller
  SET_CS;
  }
//============================================================================
// From: http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void OLED_Line(uint16_t x0, uint16_t y0,
              uint16_t x1, uint16_t y1,
              uint8_t r, uint8_t g, uint8_t b)
  {
  int16_t
    dx;
  int16_t
    sx;
  int16_t
    dy;
  int16_t
    sy;
  int16_t
    err;
  int16_t
   e2;

  //General case
  if (y0 != y1)
    {
    dx = abs((int16_t )x1 - (int16_t )x0);
    sx = x0 < x1 ? 1 : -1;
    dy = abs((int16_t )y1 - (int16_t )y0);
    sy = y0 < y1 ? 1 : -1;
    err = (dx > dy ? dx : -dy) / 2;

    for (;;)
      {
      Put_Pixel(x0, y0, r,g,b);
      if ((x0 == x1) && (y0 == y1))
        break;
      e2 = err;
      if (e2 > -dx)
        {
        err -= dy;
        x0 = (uint16_t)((int16_t) x0 + sx);
        }
      if (e2 < dy)
        {
        err += dx;
        y0 = (uint16_t)((int16_t) y0 + sy);
        }
      }
    }
  else
    {
    //Optimized for OLED
    Fast_Horizontal_Line(x0, y0, x1,r,g,b);
    }
  }
//============================================================================
#if(1)
// This function transfers data, in one stream. Slightly
// optimized to do index operations during SPI transfers.
// 312uS ~ 319uS
void SPI_send_pixels(uint8_t pixel_count, uint8_t *data_ptr)
  {
  uint8_t
    r;
  uint8_t
    g;
  uint8_t
    b;
  uint8_t
    first_half;
  uint8_t
    second_half;

  // Select the OLED's data register
  SET_RS;
  // Select the OLED controller
  CLR_CS;

  //Load the first pixel. BMPs BGR format
  b=*data_ptr;
  data_ptr++;
  g=*data_ptr;
  data_ptr++;
  r=*data_ptr;
  data_ptr++;

  //The display takes two bytes (565) RRRRR GGGGGG BBBBB
  //to show one pixel.
  first_half=(r&0xF8) | (g >> 5);
  second_half=(g<<5) | (b >> 3);

  while(pixel_count)
    {
    //Send the first half of this pixel out
    //SPDR = first_half;
    SPI.transfer(first_half);
    //Load the next pixel while that is transmitting
    b=*data_ptr;
    data_ptr++;
    g=*data_ptr;
    data_ptr++;
    r=*data_ptr;
    data_ptr++;
    //Calculate the next first half while that is transmitting
    // ~1.9368us -0.1256 us = 1.8112uS
    first_half=(r&0xF8) | (g >> 5);
    //At 8MHz SPI clock, the transfer is done by now, so we
    //do not need this:
    // while (!(SPSR & _BV(SPIF))) ;
    //Send the second half of the this pixel out
    //SPDR = second_half;
    SPI.transfer(second_half);
    //Calculate the next second half
    second_half=(g<<5) | (b >> 3);
    //Done with this pixel
    pixel_count--;
    //At 8MHz SPI clock, the transfer is done by now, so we
    //do not need this:
    // while (!(SPSR & _BV(SPIF))) ;
    }
  //Wait for the final transfer to complete before we bang on CS.
//  while (!(SPSR & _BV(SPIF))) ;
  // Deselect the OLED controller
  SET_CS;
  }
#else
// Simple. Slower
// This function transfers data, in one stream.
// 660uS ~ 667uS
void SPI_send_pixels(uint8_t pixel_count, uint8_t *data_ptr)
  {
  uint8_t
    r;
  uint8_t
    g;
  uint8_t
    b;

  //  SPI_sendCommand(LD7138_0x0C_DATA_WRITE_READ);
  while(pixel_count)
    {
    //Load the first pixel. BMPs BGR format
    b=*data_ptr;
    data_ptr++;
    g=*data_ptr;
    data_ptr++;
    r=*data_ptr;
    data_ptr++;

    //The display takes two bytes (565) RRRRR GGGGGG BBBBB
    //two show one pixel.
    //Calculate the first byte
    //RRRR RGGG
    SPI_sendData((r&0xF8) | (g >> 5));
    //Calculate the second byte
    //GGGB BBBB
    SPI_sendData((g<<5) | (b >> 3));
    //Done with this pixel
    pixel_count--;
    }
  }
#endif


static const unsigned char font[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
  0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
  0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
  0x18, 0x3C, 0x7E, 0x3C, 0x18,
  0x1C, 0x57, 0x7D, 0x57, 0x1C,
  0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
  0x00, 0x18, 0x3C, 0x18, 0x00,
  0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
  0x00, 0x18, 0x24, 0x18, 0x00,
  0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
  0x30, 0x48, 0x3A, 0x06, 0x0E,
  0x26, 0x29, 0x79, 0x29, 0x26,
  0x40, 0x7F, 0x05, 0x05, 0x07,
  0x40, 0x7F, 0x05, 0x25, 0x3F,
  0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
  0x7F, 0x3E, 0x1C, 0x1C, 0x08,
  0x08, 0x1C, 0x1C, 0x3E, 0x7F,
  0x14, 0x22, 0x7F, 0x22, 0x14,
  0x5F, 0x5F, 0x00, 0x5F, 0x5F,
  0x06, 0x09, 0x7F, 0x01, 0x7F,
  0x00, 0x66, 0x89, 0x95, 0x6A,
  0x60, 0x60, 0x60, 0x60, 0x60,
  0x94, 0xA2, 0xFF, 0xA2, 0x94,
  0x08, 0x04, 0x7E, 0x04, 0x08,
  0x10, 0x20, 0x7E, 0x20, 0x10,
  0x08, 0x08, 0x2A, 0x1C, 0x08,
  0x08, 0x1C, 0x2A, 0x08, 0x08,
  0x1E, 0x10, 0x10, 0x10, 0x10,
  0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
  0x30, 0x38, 0x3E, 0x38, 0x30,
  0x06, 0x0E, 0x3E, 0x0E, 0x06,
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x5F, 0x00, 0x00,
  0x00, 0x07, 0x00, 0x07, 0x00,
  0x14, 0x7F, 0x14, 0x7F, 0x14,
  0x24, 0x2A, 0x7F, 0x2A, 0x12,
  0x23, 0x13, 0x08, 0x64, 0x62,
  0x36, 0x49, 0x56, 0x20, 0x50,
  0x00, 0x08, 0x07, 0x03, 0x00,
  0x00, 0x1C, 0x22, 0x41, 0x00,
  0x00, 0x41, 0x22, 0x1C, 0x00,
  0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
  0x08, 0x08, 0x3E, 0x08, 0x08,
  0x00, 0x80, 0x70, 0x30, 0x00,
  0x08, 0x08, 0x08, 0x08, 0x08,
  0x00, 0x00, 0x60, 0x60, 0x00,
  0x20, 0x10, 0x08, 0x04, 0x02,
  0x3E, 0x51, 0x49, 0x45, 0x3E,
  0x00, 0x42, 0x7F, 0x40, 0x00,
  0x72, 0x49, 0x49, 0x49, 0x46,
  0x21, 0x41, 0x49, 0x4D, 0x33,
  0x18, 0x14, 0x12, 0x7F, 0x10,
  0x27, 0x45, 0x45, 0x45, 0x39,
  0x3C, 0x4A, 0x49, 0x49, 0x31,
  0x41, 0x21, 0x11, 0x09, 0x07,
  0x36, 0x49, 0x49, 0x49, 0x36,
  0x46, 0x49, 0x49, 0x29, 0x1E,
  0x00, 0x00, 0x14, 0x00, 0x00,
  0x00, 0x40, 0x34, 0x00, 0x00,
  0x00, 0x08, 0x14, 0x22, 0x41,
  0x14, 0x14, 0x14, 0x14, 0x14,
  0x00, 0x41, 0x22, 0x14, 0x08,
  0x02, 0x01, 0x59, 0x09, 0x06,
  0x3E, 0x41, 0x5D, 0x59, 0x4E,
  0x7C, 0x12, 0x11, 0x12, 0x7C,
  0x7F, 0x49, 0x49, 0x49, 0x36,
  0x3E, 0x41, 0x41, 0x41, 0x22,
  0x7F, 0x41, 0x41, 0x41, 0x3E,
  0x7F, 0x49, 0x49, 0x49, 0x41,
  0x7F, 0x09, 0x09, 0x09, 0x01,
  0x3E, 0x41, 0x41, 0x51, 0x73,
  0x7F, 0x08, 0x08, 0x08, 0x7F,
  0x00, 0x41, 0x7F, 0x41, 0x00,
  0x20, 0x40, 0x41, 0x3F, 0x01,
  0x7F, 0x08, 0x14, 0x22, 0x41,
  0x7F, 0x40, 0x40, 0x40, 0x40,
  0x7F, 0x02, 0x1C, 0x02, 0x7F,
  0x7F, 0x04, 0x08, 0x10, 0x7F,
  0x3E, 0x41, 0x41, 0x41, 0x3E,
  0x7F, 0x09, 0x09, 0x09, 0x06,
  0x3E, 0x41, 0x51, 0x21, 0x5E,
  0x7F, 0x09, 0x19, 0x29, 0x46,
  0x26, 0x49, 0x49, 0x49, 0x32,
  0x03, 0x01, 0x7F, 0x01, 0x03,
  0x3F, 0x40, 0x40, 0x40, 0x3F,
  0x1F, 0x20, 0x40, 0x20, 0x1F,
  0x3F, 0x40, 0x38, 0x40, 0x3F,
  0x63, 0x14, 0x08, 0x14, 0x63,
  0x03, 0x04, 0x78, 0x04, 0x03,
  0x61, 0x59, 0x49, 0x4D, 0x43,
  0x00, 0x7F, 0x41, 0x41, 0x41,
  0x02, 0x04, 0x08, 0x10, 0x20,
  0x00, 0x41, 0x41, 0x41, 0x7F,
  0x04, 0x02, 0x01, 0x02, 0x04,
  0x40, 0x40, 0x40, 0x40, 0x40,
  0x00, 0x03, 0x07, 0x08, 0x00,
  0x20, 0x54, 0x54, 0x78, 0x40,
  0x7F, 0x28, 0x44, 0x44, 0x38,
  0x38, 0x44, 0x44, 0x44, 0x28,
  0x38, 0x44, 0x44, 0x28, 0x7F,
  0x38, 0x54, 0x54, 0x54, 0x18,
  0x00, 0x08, 0x7E, 0x09, 0x02,
  0x18, 0xA4, 0xA4, 0x9C, 0x78,
  0x7F, 0x08, 0x04, 0x04, 0x78,
  0x00, 0x44, 0x7D, 0x40, 0x00,
  0x20, 0x40, 0x40, 0x3D, 0x00,
  0x7F, 0x10, 0x28, 0x44, 0x00,
  0x00, 0x41, 0x7F, 0x40, 0x00,
  0x7C, 0x04, 0x78, 0x04, 0x78,
  0x7C, 0x08, 0x04, 0x04, 0x78,
  0x38, 0x44, 0x44, 0x44, 0x38,
  0xFC, 0x18, 0x24, 0x24, 0x18,
  0x18, 0x24, 0x24, 0x18, 0xFC,
  0x7C, 0x08, 0x04, 0x04, 0x08,
  0x48, 0x54, 0x54, 0x54, 0x24,
  0x04, 0x04, 0x3F, 0x44, 0x24,
  0x3C, 0x40, 0x40, 0x20, 0x7C,
  0x1C, 0x20, 0x40, 0x20, 0x1C,
  0x3C, 0x40, 0x30, 0x40, 0x3C,
  0x44, 0x28, 0x10, 0x28, 0x44,
  0x4C, 0x90, 0x90, 0x90, 0x7C,
  0x44, 0x64, 0x54, 0x4C, 0x44,
  0x00, 0x08, 0x36, 0x41, 0x00,
  0x00, 0x00, 0x77, 0x00, 0x00,
  0x00, 0x41, 0x36, 0x08, 0x00,
  0x02, 0x01, 0x02, 0x04, 0x02,
  0x3C, 0x26, 0x23, 0x26, 0x3C,
  0x1E, 0xA1, 0xA1, 0x61, 0x12,
  0x3A, 0x40, 0x40, 0x20, 0x7A,
  0x38, 0x54, 0x54, 0x55, 0x59,
  0x21, 0x55, 0x55, 0x79, 0x41,
  0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
  0x21, 0x55, 0x54, 0x78, 0x40,
  0x20, 0x54, 0x55, 0x79, 0x40,
  0x0C, 0x1E, 0x52, 0x72, 0x12,
  0x39, 0x55, 0x55, 0x55, 0x59,
  0x39, 0x54, 0x54, 0x54, 0x59,
  0x39, 0x55, 0x54, 0x54, 0x58,
  0x00, 0x00, 0x45, 0x7C, 0x41,
  0x00, 0x02, 0x45, 0x7D, 0x42,
  0x00, 0x01, 0x45, 0x7C, 0x40,
  0x7D, 0x12, 0x11, 0x12, 0x7D, // A-umlaut
  0xF0, 0x28, 0x25, 0x28, 0xF0,
  0x7C, 0x54, 0x55, 0x45, 0x00,
  0x20, 0x54, 0x54, 0x7C, 0x54,
  0x7C, 0x0A, 0x09, 0x7F, 0x49,
  0x32, 0x49, 0x49, 0x49, 0x32,
  0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
  0x32, 0x4A, 0x48, 0x48, 0x30,
  0x3A, 0x41, 0x41, 0x21, 0x7A,
  0x3A, 0x42, 0x40, 0x20, 0x78,
  0x00, 0x9D, 0xA0, 0xA0, 0x7D,
  0x3D, 0x42, 0x42, 0x42, 0x3D, // O-umlaut
  0x3D, 0x40, 0x40, 0x40, 0x3D,
  0x3C, 0x24, 0xFF, 0x24, 0x24,
  0x48, 0x7E, 0x49, 0x43, 0x66,
  0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
  0xFF, 0x09, 0x29, 0xF6, 0x20,
  0xC0, 0x88, 0x7E, 0x09, 0x03,
  0x20, 0x54, 0x54, 0x79, 0x41,
  0x00, 0x00, 0x44, 0x7D, 0x41,
  0x30, 0x48, 0x48, 0x4A, 0x32,
  0x38, 0x40, 0x40, 0x22, 0x7A,
  0x00, 0x7A, 0x0A, 0x0A, 0x72,
  0x7D, 0x0D, 0x19, 0x31, 0x7D,
  0x26, 0x29, 0x29, 0x2F, 0x28,
  0x26, 0x29, 0x29, 0x29, 0x26,
  0x30, 0x48, 0x4D, 0x40, 0x20,
  0x38, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x38,
  0x2F, 0x10, 0xC8, 0xAC, 0xBA,
  0x2F, 0x10, 0x28, 0x34, 0xFA,
  0x00, 0x00, 0x7B, 0x00, 0x00,
  0x08, 0x14, 0x2A, 0x14, 0x22,
  0x22, 0x14, 0x2A, 0x14, 0x08,
  0x55, 0x00, 0x55, 0x00, 0x55, // #176 (25% block) missing in old code
  0xAA, 0x55, 0xAA, 0x55, 0xAA, // 50% block
  0xFF, 0x55, 0xFF, 0x55, 0xFF, // 75% block
  0x00, 0x00, 0x00, 0xFF, 0x00,
  0x10, 0x10, 0x10, 0xFF, 0x00,
  0x14, 0x14, 0x14, 0xFF, 0x00,
  0x10, 0x10, 0xFF, 0x00, 0xFF,
  0x10, 0x10, 0xF0, 0x10, 0xF0,
  0x14, 0x14, 0x14, 0xFC, 0x00,
  0x14, 0x14, 0xF7, 0x00, 0xFF,
  0x00, 0x00, 0xFF, 0x00, 0xFF,
  0x14, 0x14, 0xF4, 0x04, 0xFC,
  0x14, 0x14, 0x17, 0x10, 0x1F,
  0x10, 0x10, 0x1F, 0x10, 0x1F,
  0x14, 0x14, 0x14, 0x1F, 0x00,
  0x10, 0x10, 0x10, 0xF0, 0x00,
  0x00, 0x00, 0x00, 0x1F, 0x10,
  0x10, 0x10, 0x10, 0x1F, 0x10,
  0x10, 0x10, 0x10, 0xF0, 0x10,
  0x00, 0x00, 0x00, 0xFF, 0x10,
  0x10, 0x10, 0x10, 0x10, 0x10,
  0x10, 0x10, 0x10, 0xFF, 0x10,
  0x00, 0x00, 0x00, 0xFF, 0x14,
  0x00, 0x00, 0xFF, 0x00, 0xFF,
  0x00, 0x00, 0x1F, 0x10, 0x17,
  0x00, 0x00, 0xFC, 0x04, 0xF4,
  0x14, 0x14, 0x17, 0x10, 0x17,
  0x14, 0x14, 0xF4, 0x04, 0xF4,
  0x00, 0x00, 0xFF, 0x00, 0xF7,
  0x14, 0x14, 0x14, 0x14, 0x14,
  0x14, 0x14, 0xF7, 0x00, 0xF7,
  0x14, 0x14, 0x14, 0x17, 0x14,
  0x10, 0x10, 0x1F, 0x10, 0x1F,
  0x14, 0x14, 0x14, 0xF4, 0x14,
  0x10, 0x10, 0xF0, 0x10, 0xF0,
  0x00, 0x00, 0x1F, 0x10, 0x1F,
  0x00, 0x00, 0x00, 0x1F, 0x14,
  0x00, 0x00, 0x00, 0xFC, 0x14,
  0x00, 0x00, 0xF0, 0x10, 0xF0,
  0x10, 0x10, 0xFF, 0x10, 0xFF,
  0x14, 0x14, 0x14, 0xFF, 0x14,
  0x10, 0x10, 0x10, 0x1F, 0x00,
  0x00, 0x00, 0x00, 0xF0, 0x10,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
  0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0xFF,
  0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
  0x38, 0x44, 0x44, 0x38, 0x44,
  0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
  0x7E, 0x02, 0x02, 0x06, 0x06,
  0x02, 0x7E, 0x02, 0x7E, 0x02,
  0x63, 0x55, 0x49, 0x41, 0x63,
  0x38, 0x44, 0x44, 0x3C, 0x04,
  0x40, 0x7E, 0x20, 0x1E, 0x20,
  0x06, 0x02, 0x7E, 0x02, 0x02,
  0x99, 0xA5, 0xE7, 0xA5, 0x99,
  0x1C, 0x2A, 0x49, 0x2A, 0x1C,
  0x4C, 0x72, 0x01, 0x72, 0x4C,
  0x30, 0x4A, 0x4D, 0x4D, 0x30,
  0x30, 0x48, 0x78, 0x48, 0x30,
  0xBC, 0x62, 0x5A, 0x46, 0x3D,
  0x3E, 0x49, 0x49, 0x49, 0x00,
  0x7E, 0x01, 0x01, 0x01, 0x7E,
  0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
  0x44, 0x44, 0x5F, 0x44, 0x44,
  0x40, 0x51, 0x4A, 0x44, 0x40,
  0x40, 0x44, 0x4A, 0x51, 0x40,
  0x00, 0x00, 0xFF, 0x01, 0x03,
  0xE0, 0x80, 0xFF, 0x00, 0x00,
  0x08, 0x08, 0x6B, 0x6B, 0x08,
  0x36, 0x12, 0x36, 0x24, 0x36,
  0x06, 0x0F, 0x09, 0x0F, 0x06,
  0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0x00, 0x10, 0x10, 0x00,
  0x30, 0x40, 0xFF, 0x01, 0x01,
  0x00, 0x1F, 0x01, 0x01, 0x1E,
  0x00, 0x19, 0x1D, 0x17, 0x12,
  0x00, 0x3C, 0x3C, 0x3C, 0x3C,
  0x00, 0x00, 0x00, 0x00, 0x00  // #255 NBSP
};

void draw_char_12pt7b(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t matrix[12] = {0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xCF, 0x3C, 0xF3, 0x8A, 0x20, 0x06, 0x30};
   // 0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
  for (uint8_t row = 0; row < 12; row++) 
  {
    for (size_t col = 0; col < 8; col++) 
    {
      if ((FreeSans12pt7bBitmaps[row+360] & (0x80 >> col)) == 0)//matrix[row][col]==0) FreeSans12pt7bBitmaps
      {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
/*
void drawChar(int16_t x, int16_t y, unsigned char c,
  uint16_t color, uint16_t bg, uint8_t size) {

    //if(!gfxFont) { // 'Classic' built-in font

        if((x >= _width)            || // Clip right
           (y >= _height)           || // Clip bottom
           ((x + 6 * size - 1) < 0) || // Clip left
           ((y + 8 * size - 1) < 0))   // Clip top
            return;

        //if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

        //startWrite();
        for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for(int8_t j=0; j<8; j++, line >>= 1) {
                if(line & 1) {
                    if(size == 1)
                        writePixel(x+i, y+j, color);
                    else
                        //writeFillRect(x+i*size, y+j*size, size, size, color);
                } else if(bg != color) {
                    if(size == 1)
                        writePixel(x+i, y+j, bg);
                    else
                        //writeFillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }
        if(bg != color) { // If opaque, draw vertical line for last column
            if(size == 1) writeFastVLine(x+5, y, 8, bg);
            else          //writeFillRect(x+5*size, y, size, 8*size, bg);
        }
        //endWrite();
}
*/

void lib_1(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {0,0,0,0,0,1,1,1},
    {0,0,0,0,1,1,1,1},
    {0,0,0,1,1,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}

void lib_2(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
void lib_3(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,1,1,1,0,0},
    {0,0,0,1,1,1,0,0},
    {0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
void lib_4(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,1,1,0,0},
    {1,1,0,0,1,1,0,0},
    {1,1,0,0,1,1,0,0},
    {1,1,0,0,1,1,0,0},
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {0,0,0,0,1,1,0,0},
    {0,0,0,0,1,1,0,0},
    {0,0,0,0,1,1,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
void lib_5(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,1,1,1,1,0,0},
    {0,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
void lib_6(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {0,0,0,1,1,1,1,1},
    {0,0,1,1,1,1,1,1},
    {0,1,1,1,0,0,0,0},
    {1,1,1,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,1,1,1,1,0,0},
    {1,1,1,1,1,1,0,0},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
void lib_7(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,1,1,0},
    {0,0,0,0,0,1,1,0},
    {0,0,0,0,1,1,0,0},
    {0,0,0,0,1,1,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,1,1,0,0,0,0},
    {0,0,1,1,0,0,0,0},
    {0,1,1,0,0,0,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
void lib_8(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}
void lib_9(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {0,0,1,1,1,1,0,0},
    {0,0,1,1,1,1,0,0},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {0,0,1,1,1,1,1,1},
    {0,0,1,1,1,1,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,1,1,0},
    {0,1,1,1,1,1,0,0},
    {0,1,1,1,1,0,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}

void lib_I(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,0,0,1,1,0,0,0},
    {0,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}

void lib_P(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b) {
  /* code */
  uint8_t matrix[12][8]={
    {1,1,1,1,1,1,0,0},
    {1,1,1,1,1,1,1,0},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,0,0,0,0,1,1},
    {1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
  };
  for (uint8_t row = 0; row < 12; row++) {
    for (size_t col = 0; col < 8; col++) {
      if (matrix[row][col]==0) {
        Put_Pixel(x0+col, y0+row, 0,0,0);
      }
      else
      {
        Put_Pixel(x0+col, y0+row, r,g,b);
      }
    }
  }
}

void draw_IP(uint8_t x0, uint8_t y0, uint8_t r, uint8_t g, uint8_t b, char* ip) {
  lib_I(x0,y0,r,g,b);
  x0+=9;
  lib_P(x0,y0,r,g,b);
}
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
    pinMode(OLED_CS,OUTPUT);
    pinMode(OLED_RESET,OUTPUT);
    pinMode(OLED_CD,OUTPUT);

    //Drive the ports to a reasonable starting state.
    CLR_RESET;
    CLR_RS;
    SET_CS;

    // Initialize SPI. By default the clock is 4MHz.
    SPI.begin();
    //Bump the clock to 8MHz. Appears to be the maximum.
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    //while(Serial.available()==0);
    //Initialize the OLED controller
    Initialize_OLED();
    Fill_OLED(0x00,0x00,0x00);
    //!!------------------------------OLED SETUP----------------------!!
    delay(10);
while(Serial.available() || (digitalRead(BUTTON)));
    serialFlush();

    Serial.println("Waiting until cellular system has finished booting...");
     IoTDevice.waitUntilCellularSystemIsReady();

    //delay(1000);
    Serial.println("Getting BG96 Dev Kit status...");
    GetInfo();
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
    Fill_OLED(0x00,0x00,0x00);
    delay(250);
    Fill_OLED(0x00,0x00,0xFF);
    delay(250);
    Fill_OLED(0x00,0xFF,0x00);
    delay(250);
    Fill_OLED(0x00,0xFF,0xFF);
    delay(250);
    Fill_OLED(0xFF,0x00,0x00);
    delay(250);
    Fill_OLED(0xFF,0x00,0xFF);
    delay(250);
    Fill_OLED(0xFF,0xFF,0x00);
    delay(250);
    Fill_OLED(0xFF,0xFF,0xFF);
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
      OLED_Line(63,17,
               x,0,
               r++,g--,b+=2);
      }
    for(y=0;y<36;y++)
      {
      OLED_Line(63,17,
               127,y,
               r++,g+=4,b+=2);
      }
    for(x=127;0!=x;x--)
      {
      OLED_Line(63,17,
               x,35,
               r-=3,g-=2,b-=1);
      }
    for(y=35;0!=y;y--)
      {
      OLED_Line(63,17,
               0,y,
               r+-3,g--,b++);
      }
    }

  //Circle demo
  Serial.println("Circle demo");
  Fill_OLED(0x00,0x00,0x00);
  r=0xff;
  g=0x00;
  b=0x80;
  for(i=0;i<20;i++)
    {
    for(x=2;x<114;x+=2)
      {
      OLED_Circle(x+2, 17, 2+x%15,r+-6,g-=4,b+=11);
      }
    }

  //8x8 checkerboard demo
  Serial.println("8x8 checkerboard demo");
  Fill_OLED(0x00,0x00,0x00);
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
              Put_Pixel((x<<3)+sub_x,(y<<3)+sub_y, 0x00, 0x00, 0x00);
              }
            else
              {
              Put_Pixel((x<<3)+sub_x,(y<<3)+sub_y,0xFF-((x+i)<<4), 0xFF-((x<<5)+(i<<3)), 0xFF-(y<<6));
              }
            }
          }
        }
      }
    }
*/

  r=random(0,255);
  g=random(0,255);
  b=random(0,255);

  lib_1(0,10,r,g,b);
  //draw_char_12pt7b(0,10,r,g,b);
  lib_2(10,10,r,g,b);
  lib_3(19,10,r,g,b);
  lib_4(28,10,r,g,b);
  lib_5(37,10,r,g,b);
  lib_6(46,10,r,g,b);
  lib_7(55,10,r,g,b);
  lib_8(64,10,r,g,b);
  lib_9(73,10,r,g,b);
  lib_I(82,10,r,g,b);
  lib_P(91,10,r,g,b);

  draw_IP(0,23,r,g,b,"");
  
  for (x= 0; x < 100; x++) {
    for (i = 0; i < 5; i++) {
      OLED_Line(0,i,
               x,i,
               r,g,b);
    }
    delay(1);
  }

  r=random(0,255);
  g=random(0,255);
  b=random(0,255);

  lib_1(0,10,r,g,b);
  lib_2(10,10,r,g,b);
  lib_3(19,10,r,g,b);
  lib_4(28,10,r,g,b);
  lib_5(37,10,r,g,b);
  lib_6(46,10,r,g,b);
  lib_7(55,10,r,g,b);
  lib_8(64,10,r,g,b);
  lib_9(73,10,r,g,b);
  
  for (x= 100; x > 0; x--) {
    for (i = 0; i < 5; i++) {
      OLED_Line(100,i,
               x,i,
               r,g,b);
    }
    delay(1);
  }
} // void loop()
//============================================================================

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