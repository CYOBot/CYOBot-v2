#include <Arduino.h>
#include <Wire.h>
#include "LSM6DSL.h"
#include "LSM6DSL_Orientation.h"

// --- IMU Configuration ---
LSM6DSL imu(LSM6DSL_MODE_I2C, IMU_ADDR);
Orientation orientation = {0.0, 0.0, 0.0};  // Initialize orientation values

#include <Adafruit_NeoPixel.h>
// --- NeoPixel Configuration ---
#ifndef NEO_BRAIN
#define NEO_BRAIN 15
#endif
#define LED_COUNT 33

Adafruit_NeoPixel strip(LED_COUNT, NEO_BRAIN, NEO_GRB + NEO_KHZ800);

// Mapping for 5x5 grid (0-24) to physical pixels when offset is 0
// -1 means no pixel at that position
const int8_t PIXEL_MAP_OFFSET_0[25] = {
   0,  1,  2,  3,  4,
   6,  7,  8,  9, 10,
  14, 15, 16, 17, 18,
  22, 23, 24, 25, 26,
  28, 29, 30, 31, 32
};

// --- Dice Patterns (Indices 0-24) ---
// We can't use a clean map<int, vector> like Python easily on Arduino without std library overhead, 
// so we'll implement a helper function to switch.

const int DICE_1[] = {12};
const int DICE_2[] = {0, 24};
const int DICE_3[] = {4, 12, 20};
const int DICE_4[] = {0, 4, 20, 24};
const int DICE_5[] = {0, 4, 12, 20, 24};
const int DICE_6[] = {0, 4, 10, 14, 20, 24};

struct DicePattern {
  const int* indices;
  int count;
};

DicePattern getDicePattern(int value) {
  DicePattern p;
  switch (value) {
    case 1: p.indices = DICE_1; p.count = 1; break;
    case 2: p.indices = DICE_2; p.count = 2; break;
    case 3: p.indices = DICE_3; p.count = 3; break;
    case 4: p.indices = DICE_4; p.count = 4; break;
    case 5: p.indices = DICE_5; p.count = 5; break;
    case 6: p.indices = DICE_6; p.count = 6; break;
    default: p.indices = NULL; p.count = 0; break;
  }
  return p;
}

void showDice(int value, uint8_t r, uint8_t g, uint8_t b) {
  strip.clear();
  DicePattern p = getDicePattern(value);
  
  if (p.indices != NULL) {
    for (int i = 0; i < p.count; i++) {
        int gridIndex = p.indices[i];
        if (gridIndex >= 0 && gridIndex < 25) {
            int physicalIndex = PIXEL_MAP_OFFSET_0[gridIndex];
            if (physicalIndex != -1) {
                strip.setPixelColor(physicalIndex, strip.Color(r, g, b));
            }
        }
    }
  }
  strip.show();
}

float prev_accel_x = 0;

void setup() {
  Serial.begin(9600);
  delay(2000);

  Serial.println("It starts!");
  
  if (!imu.begin()) {
      Serial.println("Failed initializing LSM6DSL");
  }
  
  // Init NeoPixel
  strip.begin();
  strip.setBrightness(20);
  strip.show();

  delay(1000);

  // flush data waiting
  while(1){
    static unsigned long lastTime = millis();
    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0;
    lastTime = currentTime;

    // Read IMU values
    float ax = imu.readFloatAccelX();
    float ay = imu.readFloatAccelY();
    float az = imu.readFloatAccelZ();

    float gx = imu.readFloatGyroX();
    float gy = imu.readFloatGyroY();
    float gz = imu.readFloatGyroZ();

    // Update orientation
    computeOrientation(imu, orientation, dt);

    // Print in Serial Plotter–friendly format
    // Each value separated by tab
    Serial.print(ax, 4);  Serial.print("\t");
    Serial.print(ay, 4);  Serial.print("\t");
    Serial.print(az, 4);  Serial.print("\t");

    Serial.print(gx, 4);  Serial.print("\t");
    Serial.print(gy, 4);  Serial.print("\t");
    Serial.print(gz, 4);  Serial.print("\t");

    Serial.print(orientation.yaw, 2);   Serial.print("\t");
    Serial.print(orientation.pitch, 2); Serial.print("\t");
    Serial.println(orientation.roll, 2);

    // If we have valid acceleration data (not zero or NaN), break the loop
    if (ax != 0.0 || ay != 0.0 || az != 0.0) {
        Serial.println("IMU data valid. Starting dice logic...");
        break;
    }

    delay(50); // ~20 Hz update rate for smoother plotting
  }
  
  // Set initial state
  prev_accel_x = imu.readFloatAccelX();
  showDice(1, 100, 20, 50);
}

void loop() {
  float accel_x = imu.readFloatAccelX();
  
  // Detect Shake
  if (abs(accel_x - prev_accel_x) > 0.3) {
      
      // Roll effect
      for (int i = 0; i < 20; i++) {
          int randVal = random(1, 7); // 1 to 6
          showDice(randVal, 200, 50, 100);
          delay(100);
      }
      
      // Settle
      int finalVal = random(1, 7);
      showDice(finalVal, 100, 20, 50);
      
      // Reset prev to current to avoid re-triggering immediately on settle
      accel_x = imu.readFloatAccelX(); 
  }
  
  prev_accel_x = accel_x;
  delay(5); // ~200Hz sampling (Python had 1ms sleep, but Arduino loop is fast)
}
