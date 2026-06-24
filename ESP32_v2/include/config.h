#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// 1. MICRO-ROS CONFIGURATION
// =============================================================================
// Select only one transport mode: serial or WiFi
#define MICROROS_TRANSPORT_SERIAL 

#if defined(MICROROS_TRANSPORT_SERIAL)
  #define SERIAL_BAUD_RATE 115200
#endif

// =============================================================================
// 2. ROBOT GEOMETRY & PHYSICAL CONSTANTS
// =============================================================================
// Please adjust these values based on your robot's actual measurements
#define WHEEL_DIAMETER 0.065          // Wheel diameter in meters (e.g., 65mm)
#define WHEEL_BASE 0.200              // Distance between wheel centers in meters
#define ENCODER_TICKS_PER_REV 500.0  // Ticks per full wheel revolution (taking gear ratio into account)

// =============================================================================
// 3. PIN CONFIGURATIONS (ESP32)
// =============================================================================

// --- Motor 1 (Left Motor) - BTS7960B ---
#define L_MOTOR_LPWM_PIN 0  // Left PWM control pin
#define L_MOTOR_RPWM_PIN 4  // Right PWM control pin

// --- Motor 2 (Right Motor) - BTS7960B ---
#define R_MOTOR_LPWM_PIN 2  // Left PWM control pin
#define R_MOTOR_RPWM_PIN 15  // Right PWM control pin

// --- Encoders ---
// Ensure your encoders have external pull-up resistors if you use these pins.
#define L_ENCODER_A_PIN 26
#define L_ENCODER_B_PIN 14   
#define R_ENCODER_A_PIN 27   
#define R_ENCODER_B_PIN 12   

// --- IMU (MPU6050 via I2C) ---
#define I2C_SDA_PIN 21       // Default ESP32 SDA
#define I2C_SCL_PIN 22       // Default ESP32 SCL
#define IMU_I2C_ADDRESS 0x68 // MPU6050 default I2C address (0x68 or 0x69)

// =============================================================================
// 4. PWM CONFIGURATION (ESP32 LEDC)
// =============================================================================
#define PWM_FREQ 15000       // 15kHz frequency to minimize motor whining/humming
#define PWM_RES 8            // 8-bit resolution (values from 0 to 255)
#define LOOPTIME 40

// ESP32 requires specific hardware channels for LEDC PWM generation
// #define L_MOTOR_LPWM_CHAN 0
// #define L_MOTOR_RPWM_CHAN 1
// #define R_MOTOR_LPWM_CHAN 2
// #define R_MOTOR_RPWM_CHAN 3

// =============================================================================
// 5. PID & LOOP RATES
// =============================================================================
#define MAIN_LOOP_FREQ 20    // Target frequency (Hz) for the main update loop (micro-ROS / Odometry)
#define PID_KP 1.5
#define PID_KI 0.5
#define PID_KD 0.1

#endif // CONFIG_H