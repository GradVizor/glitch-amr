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

class Encoders {
public:
    /**
     * @brief Configures GPIO pins, sets pullups, and registers change interrupts.
     * @param pin_la Left Encoder Channel A Pin.
     * @param pin_lb Left Encoder Channel B Pin.
     * @param pin_ra Right Encoder Channel A Pin.
     * @param pin_rb Right Encoder Channel B Pin.
     */
    static void begin(uint8_t pin_la, uint8_t pin_lb, uint8_t pin_ra, uint8_t pin_rb);

    /**
     * @brief Gets current absolute position of the left encoder.
     */
    static int32_t getLeftPos();

    /**
     * @brief Gets current absolute position of the right encoder.
     */
    static int32_t getRightPos();

    /**
     * @brief Resets both encoder counts and previous states to zero.
     */
    static void reset();

    /**
     * @brief Calculates tick differences since the last time update() was called.
     * @param left_diff Output variable for left tick changes.
     * @param right_diff Output variable for right tick changes.
     */
    static void update(int32_t &left_diff, int32_t &right_diff);

private:
    // GPIO Pin storage
    static uint8_t _pin_la;
    static uint8_t _pin_lb;
    static uint8_t _pin_ra;
    static uint8_t _pin_rb;

    // Volatile counts mutated inside IRAM interrupts
    static volatile int32_t _left_pos;
    static volatile int32_t _right_pos;

    // Variables for tracking tick differentials
    static int32_t _prev_left_pos;
    static int32_t _prev_right_pos;

    // Interrupt Service Routines placed in Instruction RAM
    static void IRAM_ATTR isr_left_a();
    static void IRAM_ATTR isr_left_b();
    static void IRAM_ATTR isr_right_a();
    static void IRAM_ATTR isr_right_b();
};