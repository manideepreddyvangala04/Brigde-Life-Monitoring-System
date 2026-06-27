#ifndef SENSOR_MATH_H
#define SENSOR_MATH_H

#include <math.h>
#include <Adafruit_Sensor.h>

// Applies the Complementary Filter to calculate stable structural pitch and roll
void updateStructuralTilt(sensors_event_t a, sensors_event_t g, float dt, float &pitch, float &roll, float &z_gforce) {
    
    // 1. Calculate Pitch and Roll from Accelerometer
    float accelPitch = atan2(a.acceleration.y, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180 / PI;
    float accelRoll = atan2(-a.acceleration.x, a.acceleration.z) * 180 / PI;

    // 2. Complementary Filter (Fuses Gyro and Accelerometer data)
    // Gyroscope values from Adafruit MPU6050 are in rad/s, converting to deg/s by multiplying by 180/PI
    pitch = 0.98 * (pitch + g.gyro.x * dt * 180 / PI) + 0.02 * accelPitch;
    roll  = 0.98 * (roll  + g.gyro.y * dt * 180 / PI) + 0.02 * accelRoll;

    // 3. Convert Z-axis acceleration to G-forces
    z_gforce = abs(a.acceleration.z / 9.81);
}

#endif // SENSOR_MATH_H
