#include <Adafruit_NeoPixel.h>

// Fallback definitions if not provided by Board Variant
#ifndef BUTTON0
#define BUTTON0 4
#endif

#ifndef BUTTON1
#define BUTTON1 38
#endif

#ifndef LED
#define LED 24
#endif

#ifndef NEO_BRAIN
#define NEO_BRAIN 15
#endif

#define LED_COUNT 33

Adafruit_NeoPixel strip(LED_COUNT, NEO_BRAIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  
  // Initialize Buttons
  // Using INPUT_PULLUP assumes buttons connect to Ground when pressed
  pinMode(BUTTON0, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT_PULLUP);
  
  // Initialize Onboard LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW); // Start Off
  
  // Initialize NeoMatrix
  strip.begin();
  strip.setBrightness(50);
  strip.show(); 

  Serial.println("CYOBot Button Test Initialized");
  Serial.println("Press Button 0 (Pin 4) for RED");
  Serial.println("Press Button 1 (Pin 38) for BLUE");
}

void loop() {
  // Read button states (Active LOW)
  bool b0_pressed = (digitalRead(BUTTON0) == LOW);
  bool b1_pressed = (digitalRead(BUTTON1) == LOW);
  
  if (b0_pressed) {
    Serial.println("Button 0 Pressed!");
    digitalWrite(LED, HIGH);           // Turn on onboard LED
    fillColor(strip.Color(255, 0, 0)); // Set Matrix to Red
  } 
  else if (b1_pressed) {
    Serial.println("Button 1 Pressed!");
    digitalWrite(LED, HIGH);           // Turn on onboard LED
    fillColor(strip.Color(0, 0, 255)); // Set Matrix to Blue
  } 
  else {
    digitalWrite(LED, LOW);            // Turn off onboard LED
    fillColor(strip.Color(0, 0, 0));   // Turn off Matrix
  }
  
  delay(100); // Simple debounce / loop delay
}

void fillColor(uint32_t color) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}
