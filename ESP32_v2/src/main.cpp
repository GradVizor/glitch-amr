#include <Arduino.h>
#include <micro_ros_arduino.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h> // Header for pinging and reconnection APIs

// ROS message types
#include <geometry_msgs/msg/twist.h>
#include <nav_msgs/msg/odometry.h>
#include <sensor_msgs/msg/imu.h>

// Custom modules
#include "config.h"
#include "motor_controller.h"
#include "imu_driver.h"
#include "encoder_driver.h"

// =============================================================================
// ERROR HANDLING MACROS
// =============================================================================
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return false;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


// =============================================================================
// GLOBAL INSTANCES & VARIABLES
// =============================================================================

// Hardware drivers
MotorController leftMotor(L_MOTOR_LPWM_PIN, L_MOTOR_RPWM_PIN);
MotorController rightMotor(R_MOTOR_LPWM_PIN, R_MOTOR_RPWM_PIN);
ImuDriver imu(IMU_I2C_ADDRESS);

// System timing
unsigned long prev_millis = 0;
unsigned long prev_odom_time = 0;

// Odometry state variables (2D Pose)
double x = 0.0;
double y = 0.0;
double theta = 0.0;
double vx = 0.0;
double vth = 0.0;

// Motor target and actual angular velocities (rad/s)
double target_speed_l = 0.0;
double target_speed_r = 0.0;
double speed_act_l = 0.0;
double speed_act_r = 0.0;

// PI Controller error variables
double error_sum_l = 0.0;
double error_sum_r = 0.0;

// micro-ROS Support structures
rcl_node_t node;
rclc_support_t support;
rcl_allocator_t allocator;
rclc_executor_t executor;

// Publishers & Subscribers
rcl_publisher_t odom_pub;
rcl_publisher_t imu_pub;
rcl_subscription_t cmd_sub;

// Message storage
nav_msgs__msg__Odometry odom_msg;
sensor_msgs__msg__Imu imu_msg;
geometry_msgs__msg__Twist cmd_msg;

// Reconnection state machine
enum States {
    WAITING_AGENT,
    AGENT_AVAILABLE,
    AGENT_CONNECTED,
    AGENT_DISCONNECTED
} state;

// =============================================================================
// CALLBACKS & UTILITIES
// =============================================================================

// Callback for received /cmd_vel Twist messages
void cmd_vel_callback(const void *msgin) {
    const geometry_msgs__msg__Twist * msg = (const geometry_msgs__msg__Twist *)msgin;
    
    double linear_x = msg->linear.x;
    double angular_z = msg->angular.z;

    // Inverse kinematics: target linear velocity of left & right wheels (m/s)
    double v_left_target = linear_x - (angular_z * WHEEL_BASE / 2.0);
    double v_right_target = linear_x + (angular_z * WHEEL_BASE / 2.0);

    // Convert linear target (m/s) to wheel angular velocity target (rad/s)
    target_speed_l = v_left_target / (WHEEL_DIAMETER / 2.0);
    target_speed_r = v_right_target / (WHEEL_DIAMETER / 2.0);
}

// Calculate dead reckoning raw wheel odometry
void calculate_odometry(double dt) {
    // Current linear speeds of wheels (m/s)
    double v_left  = speed_act_l * (WHEEL_DIAMETER / 2.0);
    double v_right = speed_act_r * (WHEEL_DIAMETER / 2.0);

    // Combine into linear and angular velocity of the base
    vx = (v_left + v_right) / 2.0;
    vth = (v_right - v_left) / WHEEL_BASE;

    // Integrate state over time interval dt
    double delta_x = vx * cos(theta) * dt;
    double delta_y = vx * sin(theta) * dt;
    double delta_theta = vth * dt;

    x += delta_x;
    y += delta_y;
    theta += delta_theta;

    // Keep heading (theta) within the bounds of [-PI, PI]
    if (theta > M_PI)  theta -= 2.0 * M_PI;
    if (theta < -M_PI) theta += 2.0 * M_PI;
}

// PI motor speed control
void run_motor_controllers() {
    // Compute speed error (rad/s)
    double error_l = target_speed_l - speed_act_l;
    double error_r = target_speed_r - speed_act_r;

    // Accumulate integral terms (with anti-windup clamping to prevent overshoots)
    error_sum_l = constrain(error_sum_l + error_l, -150.0, 150.0);
    error_sum_r = constrain(error_sum_r + error_r, -150.0, 150.0);

    // PI Formula: Out = Kp * e + Ki * ∫e dt
    double out_l = (PID_KP * error_l) + (PID_KI * error_sum_l);
    double out_r = (PID_KP * error_r) + (PID_KI * error_sum_r);

    int16_t pwm_l = constrain((int16_t)out_l, -255, 255);
    int16_t pwm_r = constrain((int16_t)out_r, -255, 255);

    // If zero target speed is requested, halt motors and reset integration
    if (target_speed_l == 0.0 && target_speed_r == 0.0) {
        pwm_l = 0;
        pwm_r = 0;
        error_sum_l = 0.0;
        error_sum_r = 0.0;
    }

    leftMotor.setSpeed(-pwm_l);
    rightMotor.setSpeed(pwm_r);
}

// =============================================================================
// MICRO-ROS ENTITIES LIFECYCLE
// =============================================================================

bool create_entities() {
    allocator = rcl_get_default_allocator();

    // Initialize support object
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    // Initialize node "esp32_base_node"
    RCCHECK(rclc_node_init_default(&node, "esp32_base_node", "", &support));

    // Initialize publishers
    RCCHECK(rclc_publisher_init_default(
        &odom_pub, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry),
        "wheel/odom"));

    RCCHECK(rclc_publisher_init_default(
        &imu_pub, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
        "imu/data_raw"));

    // Initialize subscription
    RCCHECK(rclc_subscription_init_default(
        &cmd_sub, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
        "cmd_vel"));

    // Initialize executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_subscription(&executor, &cmd_sub, &cmd_msg, &cmd_vel_callback, ON_NEW_DATA));

    // 1. Pose Covariance Diagonal (x, y, z, roll, pitch, yaw)
    // Small values mean high confidence, large values (99999.0) mean unmeasured/untrusted
    for(int i = 0; i < 36; i++) odom_msg.pose.covariance[i] = 0.0;
    odom_msg.pose.covariance[0]  = 0.01;     // x position uncertainty
    odom_msg.pose.covariance[7]  = 0.01;     // y position uncertainty
    odom_msg.pose.covariance[14] = 99999.0;  // z position (not measured, 2D)
    odom_msg.pose.covariance[21] = 99999.0;  // roll (not measured, 2D)
    odom_msg.pose.covariance[28] = 99999.0;  // pitch (not measured, 2D)
    odom_msg.pose.covariance[35] = 0.05;     // yaw orientation uncertainty

    // 2. Twist Covariance Diagonal (vx, vy, vz, vroll, vpitch, vyaw)
    for(int i = 0; i < 36; i++) odom_msg.twist.covariance[i] = 0.0;
    odom_msg.twist.covariance[0]  = 0.01;     // vx linear velocity uncertainty
    odom_msg.twist.covariance[7]  = 99999.0;  // vy linear velocity (differential drive cannot slip sideways)
    odom_msg.twist.covariance[14] = 99999.0;  // vz velocity
    odom_msg.twist.covariance[21] = 99999.0;  // vroll
    odom_msg.twist.covariance[28] = 99999.0;  // vpitch
    odom_msg.twist.covariance[35] = 0.05;     // vyaw angular velocity uncertainty

    // 3. IMU Accelerometer Covariance (x, y, z)
    for(int i = 0; i < 9; i++) imu_msg.linear_acceleration_covariance[i] = 0.0;
    imu_msg.linear_acceleration_covariance[0] = 0.04; // Adjust based on your sensor noise
    imu_msg.linear_acceleration_covariance[4] = 0.04;
    imu_msg.linear_acceleration_covariance[8] = 0.04;

    // 4. IMU Gyroscope Covariance (x, y, z)
    for(int i = 0; i < 9; i++) imu_msg.angular_velocity_covariance[i] = 0.0;
    imu_msg.angular_velocity_covariance[0] = 0.02;
    imu_msg.angular_velocity_covariance[4] = 0.02;
    imu_msg.angular_velocity_covariance[8] = 0.02;

    return true;
}

void destroy_entities() {
    rmw_context_t * rmw_context = rcl_context_get_rmw_context(&support.context);

    // Updated: Correct API call for destroying sessions without hanging in Humble
    (void) rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

    rcl_publisher_fini(&odom_pub, &node);
    rcl_publisher_fini(&imu_pub, &node);
    rcl_subscription_fini(&cmd_sub, &node);
    rclc_executor_fini(&executor);
    rcl_node_fini(&node);
    rclc_support_fini(&support);
}

// =============================================================================
// MAIN PROGRAM SETUP & LOOP
// =============================================================================

void setup() {
    // Configure micro-ROS serial transport
    Serial.begin(SERIAL_BAUD_RATE);
    set_microros_transports();

    // Initialize hardware modules
    leftMotor.begin();
    rightMotor.begin();
    Encoders::begin(L_ENCODER_A_PIN, L_ENCODER_B_PIN, R_ENCODER_A_PIN, R_ENCODER_B_PIN);

    // Set up and calibrate IMU
    if (imu.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
        imu.calibrate();
    }

    state = WAITING_AGENT;
}

void loop() {
    switch (state) {
        case WAITING_AGENT:
            // Attempt to ping the agent. If available, change state
            if (rmw_uros_ping_agent(100, 1) == RMW_RET_OK) {
                state = AGENT_AVAILABLE;
            }
            break;

        case AGENT_AVAILABLE:
            // Build ROS 2 entities
            if (create_entities()) {
                state = AGENT_CONNECTED;
                prev_millis = millis();
                prev_odom_time = millis();
            } else {
                state = WAITING_AGENT;
            }
            break;

        case AGENT_CONNECTED: {
            unsigned long current_millis = millis();
            
            // Check connection status periodically
            if (rmw_uros_ping_agent(50, 1) != RMW_RET_OK) {
                state = AGENT_DISCONNECTED;
                break;
            }

            // Spin the executor to check for incoming /cmd_vel messages
            rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));

            // Main hardware update and publishing loop
            if (current_millis - prev_millis >= LOOPTIME) {
                double dt = (current_millis - prev_millis) / 1000.0;
                prev_millis = current_millis;

                // 1. Get raw encoder feedback
                int32_t left_diff = 0;
                int32_t right_diff = 0;
                Encoders::update(left_diff, right_diff);

                // 2. Compute wheel angular velocity (rad/s)
                // Convert formula: (ticks_per_loop / ticks_per_rev) * 2 * PI / dt
                double ticks_to_rads = (1.0 / dt) * (2.0 * M_PI / ENCODER_TICKS_PER_REV);
                speed_act_l = left_diff * ticks_to_rads;
                speed_act_r = right_diff * ticks_to_rads;

                // 3. Compute dead-reckoning (odometry)
                calculate_odometry(dt);

                // 4. Update the closed-loop motor PI controllers
                run_motor_controllers();

                // 5. Build and Publish Odometry message
                int64_t stamp = rmw_uros_epoch_nanos();
                odom_msg.header.stamp.sec = stamp / 1000000000;
                odom_msg.header.stamp.nanosec = stamp % 1000000000;
                odom_msg.header.frame_id.data = (char*)"odom";
                odom_msg.child_frame_id.data = (char*)"base_link";

                // Pose (x, y)
                odom_msg.pose.pose.position.x = x;
                odom_msg.pose.pose.position.y = y;
                odom_msg.pose.pose.position.z = 0.0;

                // Convert Yaw heading angle (theta) to a Quaternion
                odom_msg.pose.pose.orientation.x = 0.0;
                odom_msg.pose.pose.orientation.y = 0.0;
                odom_msg.pose.pose.orientation.z = sin(theta / 2.0);
                odom_msg.pose.pose.orientation.w = cos(theta / 2.0);

                // Twist Velocities (linear vx, angular vth)
                odom_msg.twist.twist.linear.x = vx;
                odom_msg.twist.twist.angular.z = vth;

                // Publish Odometry
                rcl_publish(&odom_pub, &odom_msg, NULL);

                // 6. Build and Publish IMU message
                xyzFloat accel = imu.getAccelMS2();
                xyzFloat gyro = imu.getGyroRad();

                imu_msg.header.stamp = odom_msg.header.stamp; // Sync timestamps
                imu_msg.header.frame_id.data = (char*)"imu_link";

                // Accelerations (m/s^2)
                imu_msg.linear_acceleration.x = accel.x;
                imu_msg.linear_acceleration.y = accel.y;
                imu_msg.linear_acceleration.z = accel.z;

                // Angular velocities (rad/s)
                imu_msg.angular_velocity.x = gyro.x;
                imu_msg.angular_velocity.y = gyro.y;
                imu_msg.angular_velocity.z = gyro.z;

                // Publish IMU data
                rcl_publish(&imu_pub, &imu_msg, NULL);
            }
            break;
        }

        case AGENT_DISCONNECTED:
            // Safe fallback: Stop the motors and clean up memory before trying to reconnect
            leftMotor.stop();
            rightMotor.stop();
            destroy_entities();
            state = WAITING_AGENT;
            break;
    }
}