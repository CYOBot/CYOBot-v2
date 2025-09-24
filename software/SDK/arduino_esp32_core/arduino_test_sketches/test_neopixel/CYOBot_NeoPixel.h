#ifndef CYOBOT_NEOPIXEL_H
#define CYOBOT_NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include <list>

#define LED_COUNT         33
#define MAX_CHAR_INDICES  25
#define MAX_PIXEL_LIST    25
#define ROWS              5

typedef struct 
{
  char character;
  uint16_t bitmap[ROWS];
} Char_Bitmap;

extern const Char_Bitmap char_A;






#endif