#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- BLE Globals ---
BLEServer *pServer = NULL;
bool deviceConnected = false;
// Standard Nordic UART Service UUIDs
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// Global Mode Variable
uint8_t mode = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      // Restart advertising on disconnect so we can reconnect
      pServer->getAdvertising()->start(); 
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        // Just take the first char as the 'mode' for simplicity
        char c = rxValue[0];
        uint8_t data = c - '0';
        if(data < 9 && data >= 0){
          mode = data; 
          Serial.print("Mode Received via BLE: ");
          Serial.println(mode);
        }
      }
    }
};

// --- Servo & Leg Configuration ---
// Using 0x43 as per user configuration
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x43);

struct Leg {
  uint8_t upper_pin;
  uint8_t lower_pin;
  int8_t upper_orientation;
  int8_t lower_orientation;
  float upper_current;
  float lower_current;
};

Leg legs[4];

// Utility: Constrain float values
float constrain_float(float val, float min_val, float max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

// Convert angle (-90 to 90) to PulseWidth (150 to 600)
// 0 deg (center) -> 90 deg Arduino -> 375 pulse (approx)
int getPulseWidth(float angle) {
    int mapped_angle = (int)(angle + 90.0);
    return map(mapped_angle, 0, 180, 150, 600);
}

void updateServoState() {
    for(int i=0; i<4; i++) {
        pwm.setPWM(legs[i].upper_pin, 0, getPulseWidth(legs[i].upper_current));
        pwm.setPWM(legs[i].lower_pin, 0, getPulseWidth(legs[i].lower_current));
    }
}

// Emulate Python's dynamicServoAssignment with 50-step interpolation
void dynamicServoAssignment(
    float l0_u, float l0_l,
    float l1_u, float l1_l,
    float l2_u, float l2_l,
    float l3_u, float l3_l
) {
    // Store starting positions
    float start_u[4], start_l[4];
    float target_u[4] = {l0_u, l1_u, l2_u, l3_u};
    float target_l[4] = {l0_l, l1_l, l2_l, l3_l};
    
    for(int k=0; k<4; k++) {
        start_u[k] = legs[k].upper_current;
        start_l[k] = legs[k].lower_current;
    }

    // Interpolate 50 steps
    for (int i = 0; i < 50; i++) {
        float step = i * 0.02; // 0.00 to 0.98
        
        for(int k=0; k<4; k++) {
            legs[k].upper_current = constrain_float(start_u[k] - step * (start_u[k] - target_u[k]), -90, 90);
            legs[k].lower_current = constrain_float(start_l[k] - step * (start_l[k] - target_l[k]), -90, 90);
        }
        updateServoState();
        delay(2); // 2ms delay matching Python
    }
    
    // Ensure final position is set accurately
    for(int k=0; k<4; k++) {
        legs[k].upper_current = constrain_float(target_u[k], -90, 90);
        legs[k].lower_current = constrain_float(target_l[k], -90, 90);
    }
    updateServoState();
}

// Gait propagation
// gait: array of 8 floats
// order: array of 4 floats (multipliers for upper servo)
void twoPhaseGaitPropagation(const float* gait, const float* order) {
    for(int i=0; i<4; i++) {
        // Calculate targets based on Python logic
        // Leg 0
        float l0_u = order[0] * legs[0].upper_orientation * gait[((i)*2+0)%8];
        float l0_l = legs[0].lower_orientation * gait[((i)*2+1)%8];
        
        // Leg 1
        float l1_u = order[1] * legs[1].upper_orientation * gait[((i+2)*2+0)%8];
        float l1_l = legs[1].lower_orientation * gait[((i+2)*2+1)%8];
        
        // Leg 2
        float l2_u = order[2] * legs[2].upper_orientation * gait[((i)*2+0)%8];
        float l2_l = legs[2].lower_orientation * gait[((i)*2+1)%8];
        
        // Leg 3
        float l3_u = order[3] * legs[3].upper_orientation * gait[((i+2)*2+0)%8];
        float l3_l = legs[3].lower_orientation * gait[((i+2)*2+1)%8];

        dynamicServoAssignment(l0_u, l0_l, l1_u, l1_l, l2_u, l2_l, l3_u, l3_l);
    }
}

// Movements
void forward() {
    // [-20, -15, -20, +20, +30, +20, +30, -15]
    static const float gait[] = {-20, -15, -20, 20, 30, 20, 30, -15};
    static const float order[] = {1.0, 1.0, 1.0, 1.0};
    twoPhaseGaitPropagation(gait, order);
}

void turnLeft() {
    // [-30, -30, -30, 0, +30, +20, +30, 0]
    // order=[-1.0, -1.0, 1.0, 1.0]
    static const float gait[] = {-30, -30, -30, 0, 30, 20, 30, 0};
    static const float order[] = {-1.0, -1.0, 1.0, 1.0};
    twoPhaseGaitPropagation(gait, order);   
}

void turnRight() {
    // [-30, -30, -30, 0, +30, +20, +30, 0]
    // order=[1.0, 1.0, -1.0, -1.0]
    static const float gait[] = {-30, -30, -30, 0, 30, 20, 30, 0};
    static const float order[] = {1.0, 1.0, -1.0, -1.0};
    twoPhaseGaitPropagation(gait, order); 
}

void sit() {
    // Python doesn't have explicit sit in Crawler command, but lets assume flat
    // dynamicServoAssignment(0, 0, 0, 0, 0, 0, 0, 0); // Center
    // Or crouch
    dynamicServoAssignment(0, 0, 0, 0, 0, 0, 0, 0);
}

void stand() {
    // Center all
    dynamicServoAssignment(0, 0, 0, 0, 0, 0, 0, 0);
}

void initial_posture() {
    // Set all to 0
    for(int i=0; i<4; i++) {
        legs[i].upper_current = 0;
        legs[i].lower_current = 0;
    }
    updateServoState();
}

void setup() {
  Serial.begin(115200);

  // Initialize I2C and PWM
  Wire.begin(17, 18);
  pwm.begin();
  pwm.setPWMFreq(60);

  // Initialize BLE
  BLEDevice::init("CYOBOT");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  BLECharacteristic *pTxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_TX,
                                           BLECharacteristic::PROPERTY_NOTIFY
                                         );
  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection to notify...");

  // Define Leg Configuration
  // Using user provided pins and Python Orientations
  // Leg0: 4, 5. Orient -1, 1
  legs[0] = {4, 5, -1, 1, 0, 0};
  // Leg1: 6, 7. Orient -1, 1
  legs[1] = {6, 7, -1, 1, 0, 0};
  // Leg2: 11, 10. Orient 1, 1
  legs[2] = {11, 10, 1, 1, 0, 0};
  // Leg3: 0, 1. Orient 1, 1
  legs[3] = {0, 1, 1, 1, 0, 0};

  initial_posture();
  delay(1000);
}

void loop() {
  // 'mode' is updated by BLE callback
  switch (mode) {
    case 1: forward(); break;
    case 2: turnLeft(); break;
    case 3: turnRight(); break;
    case 4: break; // Backward
    case 5: sit(); break;
    case 6: stand(); break;
    default:
      // Neutral / Stand still
      delay(100);
      break;
  }
}
