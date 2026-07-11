#!/usr/bin/env python3
# Copyright 2026 Reishabh Rathore
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# @author Reishabh Rathore (reishabhrathore2003@gmail.com)

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.conditions import UnlessCondition
from launch_ros.actions import Node

ARGUMENTS = [
    DeclareLaunchArgument('namespace', default_value='',
                          description='Robot namespace'),
]


def generate_launch_description():
    
    # Configurations
    namespace = LaunchConfiguration('namespace')

    # Directories
    pkg_glitch_control = get_package_share_directory('glitch_control')
    pkg_glitch_description = get_package_share_directory('glitch_description')
    pkg_glitch_bringup = get_package_share_directory('glitch_bringup')
    
    # Paths
    ekf_config_path = PathJoinSubstitution(
        [pkg_glitch_control, 'config', 'ekf.yaml'])
    robot_description_launch_file = PathJoinSubstitution(
        [pkg_glitch_description, 'launch', 'robot_description.launch.py'])
    rplidar_launch_file = PathJoinSubstitution(
        [pkg_glitch_bringup, 'launch', 'rplidar_a2m8_launch.py'])
    
    
    # Launch configurations
    robot_description_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([robot_description_launch_file])
    )
    rplidar_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([rplidar_launch_file]),
    )
    
    # micro-ROS Serial Agent Node
    # Automatically manages connection to the ESP32 via USB
    microros_agent_node = Node(
        package='micro_ros_agent',
        executable='micro_ros_agent',
        name='micro_ros_agent',
        output='screen',
        arguments=['serial', '--dev', '/dev/esp32', '-b', '115200']
    )
    
    # EKF Local Localization Node
    ekf_node = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[
            ekf_config_path
        ],
        # Remap EKF output so downstream nodes can subscribe to /odom in both sim and hardware
        remappings=[
            ('/odometry/filtered', '/odom')
        ]
    )
    
    # Static Transform: base_link -> imu_link
    # EKF needs to know the geometric position of the IMU relative to the center of the wheels.
    # Arguments: x, y, z, yaw, pitch, roll, parent_frame, child_frame (adjust x,y,z if needed)
    static_tf_imu = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_imu_link',
        arguments=['0.0', '0.0', '0.05', '0.0', '0.0', '0.0', 'base_link', 'imu_link']
    )


    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    # Include robot description
    ld.add_action(robot_description_launch)
    ld.add_action(rplidar_launch)
    ld.add_action(ekf_node)
    ld.add_action(static_tf_imu)
    ld.add_action(microros_agent_node)
    return ld
