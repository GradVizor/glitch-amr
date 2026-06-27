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

#include "motor_controller.h"

MotorController::MotorController(uint8_t lpwm_pin, uint8_t rpwm_pin)
    : _lpwm_pin(lpwm_pin), _rpwm_pin(rpwm_pin) {}

void MotorController::begin() {
    pinMode(_lpwm_pin, OUTPUT);
    pinMode(_rpwm_pin, OUTPUT);

    // Ensure motor is stopped on startup
    stop();
}

void MotorController::setSpeed(int16_t speed) {
    // Constrain input speed to safe 8-bit limits (-255 to 255)
    speed = constrain(speed, -255, 255);

    if (speed > 0) {
        // Forward direction: write PWM to LPWM, keep RPWM low
        analogWrite(_lpwm_pin, speed);
        analogWrite(_rpwm_pin, 0);
    } 
    else if (speed < 0) {
        // Reverse direction: write absolute PWM to RPWM, keep LPWM low
        analogWrite(_lpwm_pin, 0);
        analogWrite(_rpwm_pin, -speed); 
    } 
    else {
        // Speed is 0, stop motor
        stop();
    }
}

void MotorController::stop() {
    analogWrite(_lpwm_pin, 0);
    analogWrite(_rpwm_pin, 0);
}