#include <Adafruit_NeoPixel.h>

// Pin definition for the LED Ring
#ifndef NEO_BASE
#define NEO_BASE 7
#endif

// The ring has 12 LEDs
#define LED_COUNT 12

Adafruit_NeoPixel ring(LED_COUNT, NEO_BASE, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  ring.begin();
  ring.setBrightness(50); // Set brightness to ~20%
  ring.show(); // Initialize all pixels to 'off'
  Serial.println("LED Ring Test Started");
}

void loop() {
  // 1. The 'loading' effect ported exactly from your Python script
  // Fills up the ring then clears it
  Serial.println("Effect: Linear Loading");
  loading_fill_clear(10, 20, 10, 100); 
  delay(500);

  // 2. A true 'rotating' spinner effect
  // Spins a segment around the ring
  Serial.println("Effect: Rotating Spinner");
  loading_rotating(0, 0, 50, 60);
  delay(500);
}

// Port of the Python 'loading' function:
// def loading(self, red = 10, green = 20, blue = 10, speed = 0.1):
void loading_fill_clear(uint8_t r, uint8_t g, uint8_t b, int wait_ms) {
  // Fill
  for(int i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, ring.Color(r, g, b));
    ring.show();
    delay(wait_ms);
  }
  // Clear
  for(int i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, ring.Color(0, 0, 0));
    ring.show();
    delay(wait_ms);
  }
}

// A generic rotating spinner effect
void loading_rotating(uint8_t r, uint8_t g, uint8_t b, int wait_ms) {
  // Run for 3 full revolutions
  int revolutions = 3;
  int steps = revolutions * ring.numPixels();
  
  for (int i = 0; i < steps; i++) {
    ring.clear();
    
    // Draw a "comet" tail of 3 pixels
    // Head
    int head = i % ring.numPixels();
    ring.setPixelColor(head, ring.Color(r, g, b));
    
    // Middle
    int middle = (i - 1 + ring.numPixels()) % ring.numPixels();
    ring.setPixelColor(middle, ring.Color(r/2, g/2, b/2)); // Dimmer
    
    // Tail
    int tail = (i - 2 + ring.numPixels()) % ring.numPixels();
    ring.setPixelColor(tail, ring.Color(r/5, g/5, b/5)); // Dimmest

    ring.show();
    delay(wait_ms);
  }
  ring.clear();
  ring.show();
}
