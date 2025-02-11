//#include "pins_arduino_cyobrain_v2.h"
#include <Arduino.h>
#include <Wire.h>
#include "LSM6DSL.h"

// Using I2C mode by default.
LSM6DSL imu(LSM6DSL_MODE_I2C, IMU_ADDR);

void setup() {
    Serial.begin(9600);
    delay(2000);

    Serial.println("It starts!");

    if (!imu.begin()) {
        Serial.println("Failed initializing LSM6DSL");
    }
}

void loop() {
    Serial.println("\nAccelerometer:");
    Serial.print("X = ");
    Serial.println(imu.readFloatAccelX(), 4);
    Serial.print("Y = ");
    Serial.println(imu.readFloatAccelY(), 4);
    Serial.print("Z = ");
    Serial.println(imu.readFloatAccelZ(), 4);

    Serial.println("\nGyroscope:");
    Serial.print("X = ");
    Serial.println(imu.readFloatGyroX(), 4);
    Serial.print("Y = ");
    Serial.println(imu.readFloatGyroY(), 4);
    Serial.print("Z = ");
    Serial.println(imu.readFloatGyroZ(), 4);

    Serial.println("\nThermometer:");
    Serial.print(" Degrees C = ");
    Serial.println(imu.readTemperatureC(), 4);
    Serial.print(" Degrees F = ");
    Serial.println(imu.readTemperatureF(), 4);

    delay(1000);
}
