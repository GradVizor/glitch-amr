# ESP32 micro-ROS Differential Drive Robot Controller

A high-performance, robust PlatformIO firmware for ESP32 that implements a differential drive robot controller. Powered by **micro-ROS (ROS 2 Humble)**, this repository handles hardware interfacing, closed-loop speed control, real-time wheel odometry, and inertial measurements (IMU).

---

## 🛠️ System Architecture

```
+-------------------------------------------------------------+
|                 Host PC (ROS 2 Humble)                      |
|                                                             |
|   [ ROS 2 Navigation/Teleop ] <=======> [ micro-ROS Agent ] |
+-------------------------------------------------------------+
                               ||
                 Serial Transport @ 115200 Baud
                               ||
+-------------------------------------------------------------+
|                 ESP32 Microcontroller                       |
|                                                             |
|                   [ micro-ROS Client ]                      |
|                     ^              |                        |
|    Publishes:       |              | Subscribes:            |
|    - wheel/odom     |              | - cmd_vel              |
|    - imu/data_raw   |              v                        |
|                 [ Main Control Loop @ 20Hz ]                |
|                     /              \              \         |
|                    /                \              \        |
|     Closed-loop   /   I2C Data       \   Hardware   \       |
|     PI Control   /                    \  Interrupts  \      |
|                 v                      v              v     |
|             [ Motors ]               [ IMU ]     [ Encoders]|
|            (BTS7960)             (MPU6050/6500) (Quadrature)|
+-------------------------------------------------------------+
```

## 📋 Directory Structure

* **`src/`**: Source files containing implementation logic.
  * [`main.cpp`](src/main.cpp): Core state machine, micro-ROS initialization, subscriber/publisher callbacks, and the main 20Hz execution loop.
  * [`motor_controller.cpp`](src/motor_controller.cpp): BTS7960 motor driver PWM control implementation.
  * [`encoder_driver.cpp`](src/encoder_driver.cpp): Quadrature encoder tick processing using hardware interrupts.
  * [`imu_driver.cpp`](src/imu_driver.cpp): MPU6050/6500 initialization, offset calibration, and reading.
* **`include/`**: Header declarations and configuration.
  * [`config.h`](include/config.h): Physical constants (wheel base, wheel diameter, encoder ticks), pinouts, PID parameters, and rates.
* **`platformio.ini`**: PlatformIO configuration file managing toolchains and dependencies.

---

## 🔌 Hardware Wiring & Pinout

All configurations are defined in [`include/config.h`](include/config.h). The pin layout is optimized for the ESP32:

| Component | Pin Function | ESP32 Pin | Details / Notes |
| :--- | :--- | :---: | :--- |
| **Left Motor (BTS7960)** | LPWM (Forward) | `GPIO 0` | Directional PWM control |
| | RPWM (Reverse) | `GPIO 4` | Directional PWM control |
| **Right Motor (BTS7960)**| LPWM (Forward) | `GPIO 2` | Directional PWM control |
| | RPWM (Reverse) | `GPIO 15`| Directional PWM control |
| **Left Encoder** | Phase A | `GPIO 26`| Hardware interrupt-driven |
| | Phase B | `GPIO 14`| Hardware interrupt-driven |
| **Right Encoder** | Phase A | `GPIO 27`| Hardware interrupt-driven |
| | Phase B | `GPIO 12`| Hardware interrupt-driven |
| **MPU6050/6500 IMU** | SDA | `GPIO 21`| I2C Data (Requires Pull-Up) |
| | SCL | `GPIO 22`| I2C Clock (Requires Pull-Up) |

---

## 📡 ROS 2 Interface

Once connected to the micro-ROS Agent, the ESP32 exposes the following interface:

### Subscribers
* **Topic**: `/cmd_vel` 
  * **Type**: `geometry_msgs/msg/Twist`
  * **Description**: Receives target linear velocity (\(v_x\)) and angular velocity (\(\omega_z\)). The firmware automatically decomposes these into individual wheel target velocities (rad/s) using differential drive inverse kinematics.

### Publishers
* **Topic**: `/wheel/odom`
  * **Type**: `nav_msgs/msg/Odometry`
  * **Description**: Publishes the robot's computed 2D pose (\(x, y, \theta\)) and instantaneous velocities. Heading \(\theta\) (yaw) is computed by integrating the wheel velocities and is formatted as a quaternion.
* **Topic**: `/imu/data_raw`
  * **Type**: `sensor_msgs/msg/Imu`
  * **Description**: Publishes raw accelerometer values (\(m/s^2\)) and gyroscope readings (\(rad/s\)) aligned to the `imu_link` frame.

---

## 🚀 Getting Started

### Prerequisites
1. Install [PlatformIO IDE](https://platformio.org/) (either via VS Code extension or Core CLI).
2. Install **Docker** on your host PC to run the micro-ROS Agent.

### 1. Build and Flash the Firmware
Connect your ESP32 to your host PC via USB. Then execute the following commands in the project directory:

```bash
# Compile the firmware
pio run

# Upload the firmware to ESP32
pio run --target upload

# Open the serial monitor to view debugging logs
pio device monitor
```

> [!NOTE]
> The default configuration assumes your ESP32 is on `/dev/ttyUSB0`. If your port is different, update the `upload_port` and `monitor_port` values in [`platformio.ini`](file:///home/reishabh/Documents/PlatformIO/Projects/ESP32_v2/platformio.ini).

### 2. Start the micro-ROS Agent
Run the micro-ROS serial agent inside a Docker container on your host PC (making sure the baud rate and serial device match your settings):

```bash
docker run -it --rm \
  -v /dev:/dev --privileged \
  --net=host \
  microros/micro-ros-agent:humble \
  serial --dev /dev/ttyUSB0 -b 115200
```

Once the agent starts, boot/reset your ESP32. The connection state machine will transition to `AGENT_CONNECTED`, and the publishers/subscribers will become active in your ROS 2 environment.

---

## 🎛️ Control Loop & Tuning

The firmware runs a main control loop at **20Hz** (`LOOPTIME = 40` ms) containing:
1. **Speed Calculation**: Converts encoder ticks to radians per second.
2. **Odometry Integration**: Computes Dead-Reckoning coordinates by integrating wheel velocities.
3. **PI Control**: Runs independent Proportional-Integral controllers for the left and right wheels:
   $$u(t) = K_p e(t) + K_i \int e(t) \,dt$$
   
To tune the performance, edit [`include/config.h`](file:///home/reishabh/Documents/PlatformIO/Projects/ESP32_v2/include/config.h):
* **`PID_KP`**: Increase for a faster response, decrease if the motor oscillates.
* **`PID_KI`**: Increase to eliminate steady-state error, decrease if the motor overshoots or rings.
* **`MAIN_LOOP_FREQ`**: Can be increased if higher rate command/telemetry updates are needed, but ensure your micro-ROS serial bandwidth can sustain the load.
