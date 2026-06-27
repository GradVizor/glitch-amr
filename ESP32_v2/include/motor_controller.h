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

#pragma once
#include <Arduino.h>

class MotorController {
public:
    /**
     * @brief Constructor for the BTS7960 motor driver.
     * @param lpwm_pin Left PWM control pin (determines direction/speed).
     * @param rpwm_pin Right PWM control pin (determines direction/speed).
     */
    MotorController(uint8_t lpwm_pin, uint8_t rpwm_pin);

    /**
     * @brief Initializes GPIO pins and sets starting state to stopped.
     */
    void begin();

    /**
     * @brief Sets the motor speed and direction.
     * @param speed Speed value from -255 (full reverse) to 255 (full forward). 
     *              Passing 0 stops the motor.
     */
    void setSpeed(int16_t speed);

    /**
     * @brief Stops the motor by writing 0 to both PWM pins.
     */
    void stop();

private:
    uint8_t _lpwm_pin;
    uint8_t _rpwm_pin;
};