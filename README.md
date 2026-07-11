# Glitch AMR

**Glitch** is an open-source Autonomous Mobile Robot (AMR) platform built on ROS 2 Humble, featuring a split-computing architecture that combines an ESP32 microcontroller for low-level hardware control with a Single Board Computer (SBC) for high-level autonomy and navigation.

[![ROS 2 Humble](https://img.shields.io/badge/ROS_2-Humble-blue)](https://docs.ros.org/en/humble/)
[![License: Apache-2.0](https://img.shields.io/badge/License-Apache_2.0-green.svg)](https://opensource.org/licenses/Apache-2.0)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange)](https://platformio.org/)

---

## 📋 Table of Contents

- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware Specifications](#hardware-specifications)
- [Software Stack](#software-stack)
- [Repository Structure](#repository-structure)
- [Installation](#installation)
- [Usage](#usage)
  - [Hardware Setup](#hardware-setup)
  - [Simulation](#simulation)
- [ROS 2 Interface](#ros-2-interface)
- [Configuration](#configuration)
- [Contributing](#contributing)
- [License](#license)
- [Credits & Attribution](#credits--attribution)

---

## ✨ Features

- **Split-Computing Architecture**: ESP32 handles real-time motor control and sensor interfacing via micro-ROS, while the SBC runs the full ROS 2 navigation stack
- **Differential Drive Kinematics**: Optimized for 2-wheel differential drive robots
- **Closed-Loop Velocity Control**: PI controllers for precise wheel speed regulation
- **Sensor Fusion**: EKF-based odometry fusion combining wheel encoders and IMU data
- **SLAM & Navigation**: Full Nav2 integration with SLAM Toolbox for mapping and autonomous navigation
- **Gazebo Simulation**: Complete simulation support with Ignition Gazebo
- **Real-time Odometry**: Hardware-interrupt-driven encoder processing
- **IMU Integration**: MPU6050/MPU6500 support with calibration

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    Host PC / SBC (ROS 2 Humble)                 │
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌────────────┐         │
│  │  Nav2 Stack  │    │ SLAM Toolbox │    │   Teleop   │         │
│  └──────┬───────┘    └──────┬───────┘    └──────┬─────┘         │
│         │                   │                   │               │
│         └───────────────────┼───────────────────┘               │
│                             │                                   │
│                    ┌────────▼─────────┐                         │
│                    │robot_localization│                         │
│                    │   (EKF Filter)   │                         │
│                    └────────┬─────────┘                         │
│                             │                                   │
│                    ┌────────▼─────────┐                         │
│                    │  micro-ROS Agent │                         │
│                    │ (Serial @ 115200)│                         │
│                    └────────┬─────────┘                         │
└─────────────────────────────┼───────────────────────────────────┘
                              │ Serial USB
                              │
┌─────────────────────────────▼─────────────────────────────────┐
│                    ESP32 Microcontroller                      │
│                                                               │
│  ┌──────────────┐     ┌───────────────┐     ┌──────────────┐  │
│  │  micro-ROS   │◄───►│  Main Control │◄───►│   Encoder    │  │
│  │    Client    │     │  Loop (20Hz)  │     │  Interrupts  │  │
│  └──────────────┘     └──────┬────────┘     └──────────────┘  │
│                              │                                │
│         ┌────────────────────┼────────────────────┐           │
│         │                    │                    │           │
│  ┌──────▼──────┐      ┌──────▼──────┐      ┌──────▼──────┐    │
│  │   Motors    │      │     IMU     │      │  Odometry   │    │
│  │  (BTS7960)  │      │  (MPU6050)  │      │ Integration │    │
│  └─────────────┘      └─────────────┘      └─────────────┘    │
└───────────────────────────────────────────────────────────────┘
```

### Data Flow

1. **ESP32** reads encoders and IMU, computes wheel velocities and raw odometry
2. **micro-ROS Agent** serializes data over USB and publishes to ROS 2 topics
3. **EKF Node** fuses wheel velocities and IMU yaw rates into `/odometry/filtered` (remapped to `/odom`)
4. **Nav2 Stack** consumes filtered odometry for autonomous navigation

---

## 🔧 Hardware Specifications

| Component | Specification |
|-----------|---------------|
| **Microcontroller** | ESP32 (DevKitC) |
| **Motor Drivers** | 2× BTS7960B H-Bridge |
| **Motors** | DC gearmotors with quadrature encoders |
| **IMU** | MPU6050 / MPU6500 (I2C @ 0x68) |
| **2D-LiDAR** | RPLiDAR A2M8 |
| **Communication** | USB Serial @ 115200 baud |
| **Control Frequency** | 20 Hz main loop |

### Wiring Diagram

| Component | Pin Function | ESP32 GPIO | Notes |
|-----------|-------------|------------|-------|
| **Left Motor** | LPWM (Forward) | GPIO 0 | PWM control |
| | RPWM (Reverse) | GPIO 4 | PWM control |
| **Right Motor** | LPWM (Forward) | GPIO 2 | PWM control |
| | RPWM (Reverse) | GPIO 15 | PWM control |
| **Left Encoder** | Phase A | GPIO 26 | Hardware interrupt |
| | Phase B | GPIO 14 | Hardware interrupt |
| **Right Encoder** | Phase A | GPIO 27 | Hardware interrupt |
| | Phase B | GPIO 12 | Hardware interrupt |
| **MPU6050/6500** | SDA | GPIO 21 | I2C (with pull-up) |
| | SCL | GPIO 22 | I2C (with pull-up) |

---

## 💻 Software Stack

| Layer | Technology | Purpose |
|-------|-----------|---------|
| **Firmware** | PlatformIO / C++ | ESP32 motor control, sensor drivers |
| **Middleware** | micro-ROS (ROS 2 Humble) | Embedded ROS 2 client |
| **Robot OS** | ROS 2 Humble | Navigation, localization, teleoperation |
| **Localization** | robot_localization (EKF) | Sensor fusion |
| **Navigation** | Nav2 Stack | Autonomous navigation |
| **SLAM** | SLAM Toolbox | Mapping |
| **Simulation** | Ignition Gazebo | Physics simulation |

---

## 📁 Repository Structure

```
glitch-amr/
├── ESP32_v2/                    # PlatformIO firmware for ESP32
│   ├── include/
│   │   ├── config.h             # Pin definitions, PID parameters, constants
│   │   ├── encoder_driver.h     # Quadrature encoder interface
│   │   ├── imu_driver.h         # MPU6050/6500 IMU interface
│   │   └── motor_controller.h   # BTS7960 motor driver interface
│   ├── src/
│   │   ├── main.cpp             # Main control loop, micro-ROS node
│   │   ├── encoder_driver.cpp   # Encoder tick processing
│   │   ├── imu_driver.cpp       # IMU calibration & reading
│   │   └── motor_controller.cpp # PWM motor control
│   └── platformio.ini           # PlatformIO configuration
│
├── glitch_bringup/              # Hardware launch & system bringup
│   ├── launch/
│   │   ├── bringup_real.launch.py    # Main launch file (real mode)
│   │   ├── bringup_sim.launch.py    # Main launch file (sim mode)
│   │   └── rplidar_a2m8_launch.py  # RPLidar Launch file 
│
├── glitch_control/              # Control configuration
│   ├── config/
│   │   ├── control.yaml         # Diff drive controller config
│   │   └── ekf.yaml             # EKF localization parameters
│   └── launch/
│       └── control.launch.py    # Controller manager launch
│
├── glitch_description/          # Robot description & visualization
│   ├── description/
│   │   ├── urdf/                # URDF/Xacro robot models
│   │   └── models/              # Mesh files (DAE)
│   ├── config/
│   │   ├── robot.rviz           # RViz configuration
│   │   └── model.rviz           # Model-only RViz config
│   └── launch/
│       ├── robot_description.launch.py
│       ├── view_navigation.launch.py   # RViz launch to visualize navigation
│       └── view_robot.launch.py
│
├── glitch_navigation/           # SLAM & Nav2 configurations
│   ├── config/
│   │   ├── nav2_params.yaml     # Nav2 parameter server
│   │   ├── slam.yaml            # SLAM Toolbox config
│   │   └── localization.yaml    # AMCL parameters
│   ├── launch/
│   │   ├── navigation.launch.py # Nav2 bringup
│   │   ├── nav2.launch.py       # Custom Nav2 bringup
│   │   ├── slam.launch.py       # SLAM launch
│   │   └── localization.launch.py
│   └── maps/
│       └── warehouse.pgm        # Example map
│
├── glitch_nodes/                # Custom C++ ROS 2 nodes
│   └── src/
│       └── motion_control_node.cpp  # Safety & motion control
│
├── glitch_simulator/                # Gazebo simulation
│   ├── launch/                      # Ignition Gazebo launch
│   │   ├── glitch_ign.launch.py     # Gazebo sim & robot launch 
│   │   ├── glitch_spawn.launch.py   # Robot spawn in simulation    
│   │   ├── ign.launch.py            # Gazebo sim launch 
│   │   └── ros_ign_bridge.launch.py # ROS-Ignition bridge
│   └── worlds/
│       └── warehouse.sdf           # Simulation world
│
└── resources/                   # Documentation & setup guides
    ├── DOCKER_GUIDE.md          # Docker setup instructions
    ├── USB_SETUP.md             # USB port configuration guide
    └── glitch-amr.jpeg          # Project image
```

---

## 🚀 Installation

### Prerequisites

- **ROS 2 Humble** installed on Ubuntu 22.04
- **PlatformIO IDE** (VS Code extension or CLI)
- **Docker** (for micro-ROS agent)
- **Ignition Gazebo** (for simulation)

### 1. Clone the Repository

```bash
cd ~/ros2_ws/src
git clone https://github.com/GradVizor/glitch-amr.git
cd ~/ros2_ws
rosdep install --from-paths src --ignore-src -r -y
colcon build
source install/setup.bash
```

### 2. Build ESP32 Firmware

```bash
cd ESP32_v2
pio run
```

### 3. Flash Firmware to ESP32

```bash
# Upload firmware
pio run --target upload

# Monitor serial output
pio device monitor
```

---

## 🎮 Usage

### Hardware Setup

1. **Wire the robot** according to the pinout table in the [Hardware Specifications](#hardware-specifications) section
2. **Flash the ESP32** with the firmware from `ESP32_v2/` (see [ESP32_v2/README.md](ESP32_v2/README.md) for details)
3. **Configure USB ports** for stable device mapping (see [USB Port Configuration](resources/USB_SETUP.md))
4. **Connect ESP32** to the SBC via USB
5. **Launch the robot**:

```bash
# Real hardware mode (use_sim_time:=false)
ros2 launch glitch_bringup bringup_real.launch.py
```

### Simulation

Run the complete simulation stack without physical hardware:

```bash
# Launch Ignition Gazebo with Glitch robot
ros2 launch glitch_simulator glitch_ign.launch.py use_sim_time:=true
```

### Navigation

```bash
# Start SLAM mapping
ros2 launch glitch_navigation slam.launch.py

# After driving and making up the map you need 
# to save the map using the following cmd :-
< Need to be added soon >

# Start Nav2 navigation (requires a map)
# For hardware setup add " use_sim_time:=False " at the end
ros2 launch glitch_navigation navigation.launch.py use_sim_time:=True

# To visualize mapping and navigation :-
ros2 launch glitch_description view_navigation.launch.py
```

---

## 📡 ROS 2 Interface

### Published Topics

| Topic | Type | Description |
|-------|------|-------------|
| `/cmd_vel` | `geometry_msgs/Twist` | Velocity command (linear x, angular z) |
| `/wheel/odom` | `nav_msgs/Odometry` | Raw wheel odometry from ESP32 |
| `/imu/data_raw` | `sensor_msgs/Imu` | Raw IMU accelerometer & gyroscope data |
| `/odometry/filtered` | `nav_msgs/Odometry` | EKF-fused odometry (from `robot_localization`) |
| `/tf` | `tf2_msgs/TFMessage` | Transform tree (base_link → odom → imu_link) |

### Subscribed Topics

| Topic | Type | Description |
|-------|------|-------------|
| `/cmd_vel` | `geometry_msgs/Twist` | Target velocity for differential drive |

### TF Tree

```
odom
└── base_link
    └── imu_link
    └── caster_wheel_link (if applicable)
    └── wheel_left_link
    └── wheel_right_link
    └── rplidar_link
```

---

## ⚙️ Configuration

### ESP32 Parameters

For detailed ESP32 configuration and tuning, see [ESP32_v2/README.md](ESP32_v2/README.md).

Key parameters in `ESP32_v2/include/config.h`:

```cpp
// Physical constants
#define WHEEL_BASE          0.420    // meters
#define WHEEL_DIAMETER      0.120    // meters
#define ENCODER_TICKS       1200.0   // ticks per revolution

// PID gains
#define PID_KP              2.5      // Proportional gain
#define PID_KI              0.8      // Integral gain

// Loop timing
#define MAIN_LOOP_FREQ      20       // Hz
```

### EKF Configuration (`glitch_control/config/ekf.yaml`)

- Fuses `/wheel/odom/velocity` and `/imu/data_raw`
- Publishes `/odometry/filtered`
- Configured for differential drive kinematics

### Nav2 Parameters (`glitch_navigation/config/nav2_params.yaml`)

- DWB local planner tuned for differential drive
- Costmap parameters for 2D LiDAR
- Recovery behaviors configured

---

## 🤝 Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Standards

- Follow [ROS 2 Style Guide](https://docs.ros.org/en/humble/The-ROS2-Project/Contributing/Developer-Guide.html)
- Use `ament_lint` for code quality checks
- Document new features in relevant README files
- Test on both hardware and simulation when possible

---

## 📄 License

This project is licensed under the **Apache License 2.0** - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Credits & Attribution

This repository is a derivative work inspired by and built upon the following open-source projects:

- **[TurtleBot 4 Description & Viz Packages](https://github.com/turtlebot/turtlebot4)** by Clearpath Robotics, Inc. (Licensed under Apache-2.0)
  - Robot description, URDF models, and RViz configurations
  
- **[iRobot Create 3 Packages](https://github.com/iRobotEducation/create3_sim)** by iRobot Corporation (Licensed under Apache-2.0)
  - micro-ROS integration patterns and simulation setup

### Original Authors

- **Roni Kreinin** ([rkreinin@clearpathrobotics.com](mailto:rkreinin@clearpathrobotics.com)) - TurtleBot4 description package
- **Reishabh Rathore** ([reishabhrathore2003@gmail.com](mailto:reishabhrathore2003@gmail.com)) - Glitch AMR development and integration

---

## 📞 Contact

For questions, issues, or collaborations:

- **Email**: [reishabhrathore2003@gmail.com](mailto:reishabhrathore2003@gmail.com)
- **GitHub**: [@GradVizor](https://github.com/GradVizor)

---

## 📚 Additional Documentation

- **[ESP32 Firmware Guide](ESP32_v2/README.md)** - Detailed documentation for ESP32 firmware, including hardware wiring, ROS 2 interface, and control loop tuning
- **[Docker Setup Guide](resources/DOCKER_GUIDE.md)** - Complete Docker installation and deployment instructions
- **[USB Port Configuration](resources/USB_SETUP.md)** - Persistent USB port mapping for ESP32 and RPLIDAR

---

*Last updated: July 2026*
