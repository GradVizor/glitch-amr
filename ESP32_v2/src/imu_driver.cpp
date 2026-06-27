// Copyright 2026 Reishabh Rathore
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// @author Reishabh Rathore (reishabhrathore2003@gmail.com)

#include "imu_driver.h"

ImuDriver::ImuDriver(uint8_t i2c_addr) 
    : _i2c_addr(i2c_addr), _mpu(i2c_addr) {}

bool ImuDriver::begin(uint8_t sda_pin, uint8_t scl_pin) {
    // Start I2C with the designated pins defined in config.h
    Wire.begin(sda_pin, scl_pin);
    delay(100);

    // Try communicating with the IMU
    if (!_mpu.init()) {
        return false;
    }

    // Apply the hardware configurations from your working setup
    _mpu.setSampleRateDivider(5); 
    _mpu.setAccRange(MPU6500_ACC_RANGE_2G);
    _mpu.setGyrRange(MPU6500_GYRO_RANGE_250);

    return true;
}

void ImuDriver::calibrate() {
    _mpu.autoOffsets();
}

xyzFloat ImuDriver::getAccelG() {
    return _mpu.getGValues();
}

xyzFloat ImuDriver::getAccelMS2() {
    xyzFloat rawG = getAccelG();
    xyzFloat accelMS2;
    accelMS2.x = rawG.x * G_TO_MS2;
    accelMS2.y = rawG.y * G_TO_MS2;
    accelMS2.z = rawG.z * G_TO_MS2;
    return accelMS2;
}

xyzFloat ImuDriver::getGyroDeg() {
    return _mpu.getGyrValues();
}

xyzFloat ImuDriver::getGyroRad() {
    xyzFloat rawDeg = getGyroDeg();
    xyzFloat gyroRad;
    gyroRad.x = rawDeg.x * DEG_TO_RAD;
    gyroRad.y = rawDeg.y * DEG_TO_RAD;
    gyroRad.z = rawDeg.z * DEG_TO_RAD;
    return gyroRad;
}