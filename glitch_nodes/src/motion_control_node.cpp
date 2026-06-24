// Copyright 2026 iRobot Corporation. All Rights Reserved.
// @author Reishabh Rathore (reishabhrathore2003@gmail.com)

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tf2/LinearMath/Transform.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

namespace glitch_nodes
{

class MotionControlNode : public rclcpp::Node
{
public:
  explicit MotionControlNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("motion_control", options)
  {
    // Parameters
    this->declare_parameter<double>("max_speed", 0.9);

    teleop_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "cmd_vel", rclcpp::SystemDefaultsQoS(),
      std::bind(&MotionControlNode::teleop_callback, this, std::placeholders::_1));

    // Publishers
    cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
      "diffdrive_controller/cmd_vel_unstamped", rclcpp::SystemDefaultsQoS());

    // Timer for control loop (40Hz)
    control_timer_ = this->create_wall_timer(
      std::chrono::milliseconds(25),
      std::bind(&MotionControlNode::control_robot, this));
  }

private:

  void teleop_callback(geometry_msgs::msg::Twist::ConstSharedPtr msg)
  {
    const std::lock_guard<std::mutex> lock(cmd_mutex_);
    last_teleop_cmd_ = *msg;
    last_teleop_ts_ = this->now();
    RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Data received on cmd_vel!");
  }
  

  void control_robot()
  {
    auto cmd_out = std::make_unique<geometry_msgs::msg::Twist>();
    const std::lock_guard<std::mutex> lock(cmd_mutex_);
    
    // Teleop timeout: 0.5 seconds
    if ((this->now() - last_teleop_ts_) < rclcpp::Duration::from_seconds(0.5)) {
      *cmd_out = last_teleop_cmd_;
    } else {
      // Need to fix this problem of continuously moving robot.
      // If this prints, your timeout logic is triggering because it's not receiving teleop
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, "Teleop Timeout!");
      cmd_out->linear.x = 0.0;
      cmd_out->angular.z = 0.0;
    }

    // Velocity capping
    double max_speed = this->get_parameter("max_speed").as_double();
    if (std::abs(cmd_out->linear.x) > max_speed) {
      cmd_out->linear.x = std::copysign(max_speed, cmd_out->linear.x);
    }

    cmd_vel_pub_->publish(std::move(cmd_out));
  }

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr teleop_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
  rclcpp::TimerBase::SharedPtr control_timer_;

  geometry_msgs::msg::Twist last_teleop_cmd_;
  rclcpp::Time last_teleop_ts_{0, 0, RCL_ROS_TIME};
  std::mutex cmd_mutex_;

};

}  // namespace glitch_nodes

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(glitch_nodes::MotionControlNode)
