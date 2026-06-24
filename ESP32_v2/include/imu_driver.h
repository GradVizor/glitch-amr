#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <MPU6500_WE.h>

class ImuDriver {
public:
    /**
     * @brief Constructor for the MPU6050/6500 driver wrapper.
     * @param i2c_addr I2C Address of the IMU (defaults to 0x68).
     */
    ImuDriver(uint8_t i2c_addr = 0x68);

    /**
     * @brief Initializes the custom I2C bus and verifies connection with the IMU.
     * @param sda_pin Custom SDA pin on ESP32 (defaults to 21).
     * @param scl_pin Custom SCL pin on ESP32 (defaults to 22).
     * @return true if initialization succeeded, false if connection failed.
     */
    bool begin(uint8_t sda_pin = 21, uint8_t scl_pin = 22);

    /**
     * @brief Calibrates the accelerometer and gyroscope offsets. 
     *        Ensure the sensor is kept flat and steady while running this.
     */
    void calibrate();

    /**
     * @brief Reads acceleration values directly in standard gravity unit (g).
     */
    xyzFloat getAccelG();

    /**
     * @brief Reads acceleration values converted to meters per second squared (m/s^2).
     *        (Standard unit for ROS 2 sensor_msgs/Imu messages).
     */
    xyzFloat getAccelMS2();

    /**
     * @brief Reads gyroscope angular velocity in degrees per second (deg/s).
     */
    xyzFloat getGyroDeg();

    /**
     * @brief Reads gyroscope angular velocity in radians per second (rad/s).
     *        (Standard unit for ROS 2 sensor_msgs/Imu messages).
     */
    xyzFloat getGyroRad();

private:
    uint8_t _i2c_addr;
    MPU6500_WE _mpu;

    // Conversion Constants
    static constexpr float G_TO_MS2 = 9.80665f;          // Conversion: g to m/s^2
    // static constexpr float DEG_TO_RAD = 0.01745329251f; // Conversion: degrees to radians (pi / 180)
};