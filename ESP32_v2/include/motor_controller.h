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