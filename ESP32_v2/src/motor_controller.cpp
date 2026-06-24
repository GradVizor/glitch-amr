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