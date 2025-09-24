//#include "pins_arduino_cyobrain_v2.h"
#include <Arduino.h>
#include <Wire.h>
#include "LSM6DSL.h"
#include "LSM6DSL_Orientation.h"

// Using I2C mode by default.
LSM6DSL imu(LSM6DSL_MODE_I2C, IMU_ADDR);
Orientation orientation = {0.0, 0.0, 0.0};  // Initialize orientation values

void setup() {
    Serial.begin(9600);
    delay(2000);

    Serial.println("It starts!");

    if (!imu.begin()) {
        Serial.println("Failed initializing LSM6DSL");
    }
}

void loop() {
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

    delay(50); // ~20 Hz update rate for smoother plotting
}
