#include "encoder_driver.h"

// Define static class members
uint8_t Encoders::_pin_la = 0;
uint8_t Encoders::_pin_lb = 0;
uint8_t Encoders::_pin_ra = 0;
uint8_t Encoders::_pin_rb = 0;

volatile int32_t Encoders::_left_pos = 0;
volatile int32_t Encoders::_right_pos = 0;

int32_t Encoders::_prev_left_pos = 0;
int32_t Encoders::_prev_right_pos = 0;

void Encoders::begin(uint8_t pin_la, uint8_t pin_lb, uint8_t pin_ra, uint8_t pin_rb) {
    _pin_la = pin_la;
    _pin_lb = pin_lb;
    _pin_ra = pin_ra;
    _pin_rb = pin_rb;

    // Set pin modes. Note: ESP32 Pins 34-39 do not support internal pullups, 
    // but the INPUT_PULLUP mode is safe for pins < 34.
    pinMode(_pin_la, INPUT_PULLUP);
    pinMode(_pin_lb, INPUT_PULLUP);
    pinMode(_pin_ra, INPUT_PULLUP);
    pinMode(_pin_rb, INPUT_PULLUP);

    // Register interrupts on both state changes (rising and falling)
    attachInterrupt(digitalPinToInterrupt(_pin_la), isr_left_a, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pin_lb), isr_left_b, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pin_ra), isr_right_a, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_pin_rb), isr_right_b, CHANGE);
}

int32_t Encoders::getLeftPos() {
    return _left_pos;
}

int32_t Encoders::getRightPos() {
    return _right_pos;
}

void Encoders::reset() {
    _left_pos = 0;
    _right_pos = 0;
    _prev_left_pos = 0;
    _prev_right_pos = 0;
}

void Encoders::update(int32_t &left_diff, int32_t &right_diff) {
    int32_t current_left = _left_pos;
    int32_t current_right = _right_pos;

    // Compute ticks elapsed since last call
    left_diff = current_left - _prev_left_pos;
    right_diff = current_right - _prev_right_pos;

    _prev_left_pos = current_left;
    _prev_right_pos = current_right;
}

// =============================================================================
// INTERRUPT SERVICE ROUTINES (ISRs)
// =============================================================================

void IRAM_ATTR Encoders::isr_left_a() {
    if (digitalRead(_pin_la) == HIGH) {
        if (digitalRead(_pin_lb) == LOW) {
            _left_pos++;
        } else {
            _left_pos--;
        }
    } else {
        if (digitalRead(_pin_lb) == HIGH) {
            _left_pos++;
        } else {
            _left_pos--;
        }
    }
}

void IRAM_ATTR Encoders::isr_left_b() {
    if (digitalRead(_pin_lb) == HIGH) {
        if (digitalRead(_pin_la) == HIGH) {
            _left_pos++;
        } else {
            _left_pos--;
        }
    } else {
        if (digitalRead(_pin_la) == LOW) {
            _left_pos++;
        } else {
            _left_pos--;
        }
    }
}

void IRAM_ATTR Encoders::isr_right_a() {
    if (digitalRead(_pin_ra) == HIGH) {
        if (digitalRead(_pin_rb) == LOW) {
            _right_pos--;
        } else {
            _right_pos++;
        }
    } else {
        if (digitalRead(_pin_rb) == HIGH) {
            _right_pos--;
        } else {
            _right_pos++;
        }
    }
}

void IRAM_ATTR Encoders::isr_right_b() {
    if (digitalRead(_pin_rb) == HIGH) {
        if (digitalRead(_pin_ra) == HIGH) {
            _right_pos--;
        } else {
            _right_pos++;
        }
    } else {
        if (digitalRead(_pin_ra) == LOW) {
            _right_pos--;
        } else {
            _right_pos++;
        }
    }
}