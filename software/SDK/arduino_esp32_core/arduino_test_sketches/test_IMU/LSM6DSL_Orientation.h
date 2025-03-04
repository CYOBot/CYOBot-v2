#ifndef LSM6DSL_ORIENTATION_H
#define LSM6DSL_ORIENTATION_H

#include "LSM6DSL_Library.h"

// Constants for sensor conversion
#define GYRO_SENSITIVITY 0.035      // Adjust based on your gyro range settings
#define ACCEL_SENSITIVITY 0.000061  // Adjust based on your accel range settings
#define DEG_TO_RAD 0.0174533
#define ALPHA 0.98  // Complementary filter constant

// Structure to store orientation angles
struct Orientation {
    float yaw;
    float pitch;
    float roll;
};

// Function to compute yaw, pitch, and roll
/*             Hot to Calculate DT:
    static unsigned long lastTime = millis();
    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0;
    lastTime = currentTime;
*/
void computeOrientation(LSM6DSL& imu, Orientation& orientation, float dt);

#endif  // LSM6DSL_ORIENTATION_H