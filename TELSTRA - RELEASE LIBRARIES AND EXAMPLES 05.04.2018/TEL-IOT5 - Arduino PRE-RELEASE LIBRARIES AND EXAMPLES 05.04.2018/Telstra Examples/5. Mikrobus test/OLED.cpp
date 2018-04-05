#include "OLED.h"

OLED::OLED()
{
	
}

//============================================================================
void OLED::SPI_sendCommand(uint8_t command)
{
  // Select the OLED's command register
  CLR_RS;
  // Select the OLED controller
  CLR_CS;
  //Send the command via SPI:
  SPI.transfer(command);
  // Deselect the OLED controller
  SET_CS;
}
//----------------------------------------------------------------------------
void OLED::SPI_sendData(uint8_t data)
{
  // Select the OLED's data register
  SET_RS;
  // Select the OLED controller
  CLR_CS;
  //Send the data via SPI:
  SPI.transfer(data);
  // Deselect the OLED controller
  SET_CS;
}
//============================================================================
void OLED::Set_OLED_for_write_at_X_Y(uint8_t x, uint8_t y)
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
void OLED::fill_OLED(uint8_t R, uint8_t G, uint8_t B)
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
void OLED::begin(void)
{
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
	SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
		
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

  fill_OLED(0x00,0x00,0x00);

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
void OLED::drawPixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
{
  Set_OLED_for_write_at_X_Y(x, y);
  //Write the single pixel's worth of data
  SPI_sendData((R&0xF8) | (G >> 5));
  SPI_sendData((G<<5) | (B >> 3));
}
#else //faster: 27.75uS, bigger (70 bytes)
void OLED::drawPixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
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
void OLED::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t R, uint16_t G, uint16_t B)
{
  uint16_t x = radius;
  uint16_t y = 0;
  int16_t radiusError = 1 - (int16_t) x;

  while (x >= y)
    {
    //11 O'Clock
    drawPixel(x0 - y, y0 + x, R, G, B);
    //1 O'Clock
    drawPixel(x0 + y, y0 + x, R, G, B);
    //10 O'Clock
    drawPixel(x0 - x, y0 + y, R, G, B);
    //2 O'Clock
    drawPixel(x0 + x, y0 + y, R, G, B);
    //8 O'Clock
    drawPixel(x0 - x, y0 - y, R, G, B);
    //4 O'Clock
    drawPixel(x0 + x, y0 - y, R, G, B);
    //7 O'Clock
    drawPixel(x0 - y, y0 - x, R, G, B);
    //5 O'Clock
    drawPixel(x0 + y, y0 - x, R, G, B);

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
void OLED::fastHorizontalLine(uint16_t x0, uint16_t y, uint16_t x1,
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
void OLED::drawLine(uint16_t x0, uint16_t y0,
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
      drawPixel(x0, y0, r,g,b);
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
    fastHorizontalLine(x0, y0, x1,r,g,b);
    }
}
//============================================================================
#if(1)
// This function transfers data, in one stream. Slightly
// optimized to do index operations during SPI transfers.
// 312uS ~ 319uS
void OLED::SPI_send_pixels(uint8_t pixel_count, uint8_t *data_ptr)
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
void OLED::SPI_send_pixels(uint8_t pixel_count, uint8_t *data_ptr)
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
//============================================================================
void OLED::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t r, uint8_t g, uint8_t b) 
{
  for(int16_t j = y; j < y + h; j++)
  { 
    for (int16_t i = x; i < x + w; i++) 
    {
      drawPixel(i, j, r, g, b);
    }
  }
}
//============================================================================
void OLED::drawChar(unsigned char c, int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b,
    uint8_t size) 
{
  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
      return;

  for(int8_t i = 0; i < 6; i++) // Char bitmap = 5 columns
  {
    uint8_t line;
    if(i < 5) 
    {
      line = pgm_read_byte(font+(c*5)+i);
    }
    else
    {
      line = 0x0;
    }
    
    for(int8_t j = 0; j < 8; j++, line >>= 1) 
    {
      if(line & 0x1)
      {
        if(size == 1)
        {
          drawPixel(x+i, y+j, r, g, b);
        }
        else
        {
          fillRect(x+(i*size), y+(j*size), size, size, r, g, b);
        }
      }
      else
      {
        if(size == 1)
        {
          drawPixel(x+i, y+j, 0, 0, 0);
        }
        else
        {
          fillRect(x+(i*size), y+(j*size), size, size, 0, 0, 0);
        }
      }
    }
  }
}
//============================================================================
void OLED::drawString(String str, int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b,
    uint8_t size)
{
  for(uint8_t i = 0; i < str.length(); i++)
  {
    drawChar(str[i], x, y, r, g, b, size);
    x+=(size*6);
  }
}
//============================================================================
void OLED::drawImage(const uint8_t* img, int16_t x, int16_t y, int16_t w, int16_t h)
{
  uint16_t index = 0;
  for(uint8_t j = 0; j < h; j++)
  {
    for(uint8_t i = 0; i < w; i++)
    {
      drawPixel(x + i, y + h - j - 1, img[index + 2], img[index + 1], img[index]);
      index+=3;
    } 
  }
}
//============================================================================

