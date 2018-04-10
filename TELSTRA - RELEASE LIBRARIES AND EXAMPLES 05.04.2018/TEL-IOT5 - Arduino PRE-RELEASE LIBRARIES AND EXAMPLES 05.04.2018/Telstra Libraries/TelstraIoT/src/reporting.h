#ifndef reporting_H
#define reporting_H

#include "Arduino.h"

#define REPORT_NO 0
#define REPORT_L1 1
#define REPORT_L2 2
#define REPORT_L3 3
#define REPORT_L4 4

void rep_setLevel(unsigned int level);

void report(const __FlashStringHelper *ifsh);
void report(unsigned int level, const __FlashStringHelper *ifsh);

void report(char * text);
void report(unsigned int level, char * text);

void report(long number);
void report(unsigned int level, long number);

void reportln(const __FlashStringHelper *ifsh);
void reportln(unsigned int level, const __FlashStringHelper *ifsh);

void reportln(char * text);
void reportln(unsigned int level, char * text);

void reportln(long number);
void reportln(unsigned int level, long number);

#endif