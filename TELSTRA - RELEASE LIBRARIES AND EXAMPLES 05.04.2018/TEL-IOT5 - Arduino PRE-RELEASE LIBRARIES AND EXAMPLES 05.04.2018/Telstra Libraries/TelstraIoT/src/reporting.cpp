#include "reporting.h"

volatile unsigned int report_level = REPORT_L1;

void rep_setLevel(unsigned int level)
{
	report_level = level;
}

void report(const __FlashStringHelper *ifsh)
{
	if ((report_level > REPORT_NO) && Serial) Serial.print(ifsh);
}

void report(unsigned int level, const __FlashStringHelper *ifsh)
{
	if ((report_level >= level) && Serial) Serial.print(ifsh);
}

void report(char * text)
{
	if ((report_level > REPORT_NO) && Serial) Serial.print(text);
}

void report(unsigned int level, char * text)
{
	if ((report_level >= level) && Serial) Serial.print(text);
}

void report(long number)
{
	if ((report_level > REPORT_NO) && Serial) Serial.print(number);
}

void report(unsigned int level, long number)
{
	if ((report_level >= level) && Serial) Serial.print(number);
}

void reportln(const __FlashStringHelper *ifsh) 
{
	if ((report_level > REPORT_NO) && Serial) Serial.println(ifsh);
}

void reportln(unsigned int level, const __FlashStringHelper *ifsh)
{
	if ((report_level >= level) && Serial) Serial.println(ifsh);
}

void reportln(char * text) 
{
	if ((report_level > REPORT_NO) && Serial) Serial.println(text);
}

void reportln(unsigned int level, char * text)
{
	if ((report_level >= level) && Serial) Serial.println(text);
}

void reportln(long number) 
{
	if ((report_level > REPORT_NO) && Serial) Serial.println(number);
}

void reportln(unsigned int level, long number)
{
	if ((report_level >= level) && Serial) Serial.println(number);
}