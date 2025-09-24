#include <Adafruit_NeoPixel.h>
#include "CYOBot_NeoPixel.h"
#include <list>
//#include "pins_arduino_cyobrain_v2.h"

#define LED_PIN NEO_BRAIN
#define LED_COUNT 33
#define MAX_CHAR_INDICES 25
#define MAX_PIXEL_LIST 25

Adafruit_NeoPixel matrix(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);



struct Pixel 
{
  uint8_t indices;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

struct Alphabet
{
  char character;
  uint8_t indicies[MAX_CHAR_INDICES];
  uint8_t count;
};

struct OffsetPixelList
{
  int8_t offsetIndex;
  int16_t pixels[25];
};

const struct OffsetPixelList offset_pixel_lists[] = {
    {6, {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},  // 6
    {5, {-1, -1, -1, -1, -1, -1, -1, -1, -1,  5, -1, -1, -1, 12, 13, -1, -1, -1, -1, 21, -1, -1, -1, -1, -1}},  // 5
    {4, {-1, -1, -1, -1,  0, -1, -1, -1,  5,  6, -1, -1, 12, 13, 14, -1, -1, -1, 21, 22, -1, -1, -1, -1, 28}},  // 4
    {3, {-1, -1, -1,  0,  1, -1, -1,  5,  6,  7, -1, 12, 13, 14, 15, -1, -1, 21, 22, 23, -1, -1, -1, 28, 29}},  // 3
    {2, {-1, -1,  0,  1,  2, -1,  5,  6,  7,  8, 12, 13, 14, 15, 16, -1, 21, 22, 23, 24, -1, -1, 28, 29, 30}},  // 2
    {1, {-1,  0,  1,  2,  3,  5,  6,  7,  8,  9, 13, 14, 15, 16, 17, 21, 22, 23, 24, 25, -1, 28, 29, 30, 31}},  // 1
    {0, { 0,  1,  2,  3,  4,  6,  7,  8,  9, 10, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 28, 29, 30, 31, 32}},  // 0
    {-1, { 1,  2,  3,  4, -1,  7,  8,  9, 10, 11, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27, 29, 30, 31, 32, -1}},  // -1
    {-2, { 2,  3,  4, -1, -1,  8,  9, 10, 11, -1, 16, 17, 18, 19, 20, 24, 25, 26, 27, -1, 30, 31, 32, -1, -1}},  // -2
    {-3, { 3,  4, -1, -1, -1,  9, 10, 11, -1, -1, 17, 18, 19, 20, -1, 25, 26, 27, -1, -1, 31, 32, -1, -1, -1}},  // -3
    {-4, { 4, -1, -1, -1, -1, 10, 11, -1, -1, -1, 18, 19, 20, -1, -1, 26, 27, -1, -1, -1, 32, -1, -1, -1, -1}},  // -4
    {-5, {-1, -1, -1, -1, -1, 11, -1, -1, -1, -1, 19, 20, -1, -1, -1, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1}},  // -5
    {-6, {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 20, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}   // -6
};

const Alphabet alphabet[] = {
    {'A', {1, 2, 3, 5, 9, 10, 11, 12, 13, 14, 15, 19, 20, 24}, 14},
    {'B', {0, 1, 2, 5, 8, 10, 11, 12, 13, 15, 19, 20, 21, 22, 23}, 15},
    {'C', {1, 2, 3, 5, 9, 10, 15, 19, 21, 22, 23}, 11},
    {'D', {0, 1, 2, 3, 5, 9, 10, 14, 15, 19, 20, 21, 22, 23}, 14},
    {'E', {0, 1, 2, 3, 4, 5, 10, 11, 12, 13, 15, 20, 21, 22, 23, 24}, 16},
    {'F', {0, 1, 2, 3, 4, 5, 10, 11, 12, 13, 15, 20}, 12},
    {'G', {1, 2, 3, 5, 10, 12, 13, 14, 15, 19, 21, 22, 23}, 13},
    {'H', {0, 4, 5, 9, 10, 11, 12, 13, 14, 15, 19, 20, 24}, 13},
    {'I', {1, 2, 3, 7, 12, 17, 21, 22, 23}, 9},
    {'J', {0, 1, 2, 3, 4, 8, 13, 15, 18, 21, 22}, 11},
    {'K', {0, 4, 5, 8, 10, 11, 12, 15, 18, 20, 24}, 11},
    {'L', {1, 6, 11, 16, 21, 22, 23, 24}, 8},
    {'M', {0, 4, 5, 6, 8, 9, 10, 12, 14, 15, 19, 20, 24}, 13},
    {'N', {0, 4, 5, 6, 9, 10, 12, 14, 15, 18, 19, 20, 24}, 13},
    {'O', {1, 2, 3, 5, 9, 10, 14, 15, 19, 21, 22, 23}, 12},
    {'P', {0, 1, 2, 3, 5, 9, 10, 11, 12, 13, 15, 20}, 12},
    {'Q', {1, 2, 5, 8, 10, 13, 15, 18, 21, 22, 23, 24}, 12},
    {'R', {0, 1, 2, 3, 5, 9, 10, 11, 12, 13, 15, 18, 20, 24}, 14},
    {'S', {1, 2, 3, 5, 11, 12, 13, 19, 20, 21, 22, 23}, 12},
    {'T', {0, 1, 2, 3, 4, 7, 12, 17, 22}, 9},
    {'U', {0, 4, 5, 9, 10, 14, 15, 19, 21, 22, 23}, 11},
    {'V', {0, 4, 5, 9, 10, 14, 16, 18, 22}, 9},
    {'W', {0, 4, 5, 9, 10, 12, 14, 15, 17, 19, 21, 23}, 12},
    {'X', {0, 4, 6, 8, 12, 16, 18, 20, 24}, 9},
    {'Y', {0, 4, 6, 8, 12, 17, 22}, 7},
    {'Z', {0, 1, 2, 3, 4, 8, 12, 16, 20, 21, 22, 23, 24}, 13},
    {'0', {1, 2, 3, 5, 6, 9, 10, 12, 14, 15, 18, 19, 21, 22, 23}, 15},
    {'1', {2, 6, 7, 12, 17, 21, 22, 23}, 8},
    {'2', {1, 2, 5, 8, 12, 16, 20, 21, 22, 23}, 10},
    {'3', {1, 2, 5, 8, 12, 15, 18, 21, 22}, 9},
    {'4', {3, 7, 8, 11, 13, 15, 16, 17, 18, 19, 23}, 11},
    {'5', {0, 1, 2, 3, 4, 5, 10, 11, 12, 13, 19, 20, 21, 22, 23}, 15},
    {'6', {1, 2, 3, 5, 10, 11, 12, 13, 15, 19, 21, 22, 23}, 13},
    {'7', {0, 1, 2, 3, 4, 8, 12, 16, 20}, 9},
    {'8', {1, 2, 3, 5, 9, 11, 12, 13, 15, 19, 21, 22, 23}, 13},
    {'9', {1, 2, 3, 5, 9, 11, 12, 13, 14, 19, 21, 22, 23}, 13},
    {' ', {}, 0},
    {'.', {22}, 1},
    {'_', {20, 21, 22, 23, 24}, 5},
    {'[', {0, 1, 5, 10, 15, 20, 21}, 7},
    {']', {3, 4, 9, 14, 19, 23, 24}, 7},
    {'!', {2, 7, 12, 22}, 4},
    {'<', {4, 7, 10, 17, 24}, 5},
    {'>', {0, 7, 14, 17, 20}, 5},
    {',', {17, 21}, 2},
    {'\'', {2, 7}, 2},
    {(char)3, {5, 1, 6, 7, 3, 8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 22}, 16},  // Heart symbol
    {':', {7, 17}, 2}
};

const int ALPHABET_SIZE = sizeof(alphabet) / sizeof(alphabet[0]);



void pixel_init(Pixel* pixel, uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
  pixel->indices = index;
  pixel->red = red;
  pixel->green = green;
  pixel->blue = blue;
}

void pixel_set(Pixel* pixel, int16_t red, int16_t green, int16_t blue)
{
  if (red != -1 && red != pixel->red)
  {
    pixel->red = (uint8_t) red;
  }
  if (green != -1 && green != pixel->green)
  {
    pixel->green = (uint8_t) green;
  }
  if (blue != -1 && blue != pixel->blue)
  {
    pixel->blue = (uint8_t) blue;
  }
}

void pixel_get_color(Pixel* pixel1, Pixel* pixel2)
{
  pixel1->red = pixel2->red;
  pixel1->green = pixel2->green;
  pixel1->blue = pixel2->blue;
}

void init_matrix()
{
  matrix.begin();
  matrix.setBrightness(100);
  matrix.show(); // Initialize all pixels to 'off'
}

void clear_matrix()
{
  matrix.clear();
}

void reset_matrix()
{
  for(int i = 0; i < LED_COUNT; i++)
  {
    matrix.setPixelColor(i, matrix.Color(0, 0, 0));
  }
  matrix.show();
}

void set_all_matrix(Pixel pixel)
{
  for(int i = 0; i < LED_COUNT; i++)
  {
    matrix.setPixelColor(i, matrix.Color(pixel.red, pixel.green, pixel.blue));
  }
  matrix.show();
}

void set_manual_matrix(Pixel pixel)
{
  matrix.setPixelColor(pixel.indices, matrix.Color(pixel.red, pixel.green, pixel.blue));
  matrix.show();
}

//void set_character(char c, int offset = 0, Pixel pixel = {5, 5, 5}, bool multiplex = false){
//   uint8_t* test_nested_list[9] = {};
//   uint8_t test[5] = {1, 2, 3, 4, 5};
//   test_nested_list[0] = &test;
//}

void setup() 
{
  Serial.begin(9600);
  init_matrix();
}

void loop()
{
  Pixel pixel;
  pixel_init(&pixel, 0, 0, 0, 0);
  pixel_set(&pixel,100, 0, 20);
//  Pixel pixel = {10, 20, 10};
  set_all_matrix(pixel);
  delay(5000);
  //reset_matrix();
  clear_matrix();
  delay(5000);
  set_manual_matrix(pixel);
  delay(5000);
  pixel_set(&pixel,100, 0, 255);
  set_all_matrix(pixel);
  delay(5000);
}

