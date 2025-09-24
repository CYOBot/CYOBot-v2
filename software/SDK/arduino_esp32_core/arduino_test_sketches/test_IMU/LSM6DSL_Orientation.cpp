#include "LSM6DSL_Orientation.h"
#include <math.h>

// Function to compute yaw, pitch, and roll
void computeOrientation(LSM6DSL& imu, Orientation& orientation, float dt) {
    // Read accelerometer data
    float accelX = imu.readFloatAccelX();
    float accelY = imu.readFloatAccelY();
    float accelZ = imu.readFloatAccelZ();

    // Read gyroscope data
    float gyroX = imu.readFloatGyroX();
    float gyroY = imu.readFloatGyroY();
    float gyroZ = imu.readFloatGyroZ();

    // Convert raw sensor data
    gyroX *= GYRO_SENSITIVITY;
    gyroY *= GYRO_SENSITIVITY;
    gyroZ *= GYRO_SENSITIVITY;

    accelX *= ACCEL_SENSITIVITY;
    accelY *= ACCEL_SENSITIVITY;
    accelZ *= ACCEL_SENSITIVITY;

    // Compute pitch and roll from accelerometer
    float accelPitch = atan2(accelY, sqrt(accelX * accelX + accelZ * accelZ)) * 180.0 / M_PI;
    float accelRoll = atan2(-accelX, accelZ) * 180.0 / M_PI;

    // Apply complementary filter
    orientation.pitch = ALPHA * (orientation.pitch + gyroX * dt) + (1 - ALPHA) * accelPitch;
    orientation.roll = ALPHA * (orientation.roll + gyroY * dt) + (1 - ALPHA) * accelRoll;
    orientation.yaw += gyroZ * dt;  // No absolute reference for yaw
}
