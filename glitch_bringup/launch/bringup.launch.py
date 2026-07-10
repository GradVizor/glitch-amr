#!/usr/bin/env python3
# Copyright 2021 iRobot Corporation. All Rights Reserved.
# @author Rodrigo Jose Causarano Nunez (rcausaran@irobot.com)
#
# Launch Create(R) 3 nodes
# Modified by Reishabh Rathore <reishabhrathore2003@gmail.com> (2026)

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.conditions import UnlessCondition
from launch_ros.actions import Node

ARGUMENTS = [
    # Defaults to 'false' for physical hardware. Pass 'true' when launching simulation.
    DeclareLaunchArgument('use_sim_time', default_value='false',
                          choices=['true', 'false'],
                          description='Use simulation clock if true'),
    DeclareLaunchArgument('gazebo', default_value='classic',
                          choices=['classic', 'ignition'],
                          description='Which gazebo simulator to use'),
    DeclareLaunchArgument('namespace', default_value='',
                          description='Robot namespace'),
]


def generate_launch_description():
    
    # Configurations
    use_sim_time = LaunchConfiguration('use_sim_time')
    namespace = LaunchConfiguration('namespace')

    # Directories
    pkg_glitch_control = get_package_share_directory('glitch_control')
    pkg_glitch_nodes = 'glitch_nodes'
    
    # Paths
    control_launch_file = PathJoinSubstitution(
        [pkg_glitch_control, 'launch', 'control.launch.py'])
    
    ekf_config_path = PathJoinSubstitution(
        [pkg_glitch_control, 'config', 'ekf.yaml'])

    # Includes
    diffdrive_controller = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([control_launch_file]),
        launch_arguments=[('namespace', LaunchConfiguration('namespace'))]
    )

    # Motion Control
    motion_control_node = Node(
        package=pkg_glitch_nodes,
        name='motion_control',
        executable='motion_control',
        parameters=[{
            'use_sim_time': True,
            'safety_override': 'backup_only'
        }],
        output='screen',
        remappings=[
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static')
        ]
    )
    
    # micro-ROS Serial Agent Node
    # Automatically manages connection to the ESP32 via USB
    microros_agent_node = Node(
        package='micro_ros_agent',
        executable='micro_ros_agent',
        name='micro_ros_agent',
        output='screen',
        arguments=['serial', '--dev', '/dev/ttyUSB0', '-b', '115200'],
        parameters=[{'use_sim_time': use_sim_time}],
        condition=UnlessCondition(use_sim_time)
    )
    
    # EKF Local Localization Node
    ekf_node = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[
            ekf_config_path,
            {'use_sim_time': use_sim_time}
        ],
        # Remap EKF output so downstream nodes can subscribe to /odom in both sim and hardware
        remappings=[
            ('/odometry/filtered', '/odom')
        ],
        # This node will not launch if use_sim_time is true
        condition=UnlessCondition(use_sim_time)
    )
    
    # Static Transform: base_link -> imu_link
    # EKF needs to know the geometric position of the IMU relative to the center of the wheels.
    # Arguments: x, y, z, yaw, pitch, roll, parent_frame, child_frame (adjust x,y,z if needed)
    static_tf_imu = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='base_link_to_imu_link',
        arguments=['0.0', '0.0', '0.05', '0.0', '0.0', '0.0', 'base_link', 'imu_link'],
            parameters=[{'use_sim_time': use_sim_time}],
        # This node will not launch if use_sim_time is true
        condition=UnlessCondition(use_sim_time)
    )


    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    # Include robot description
    ld.add_action(diffdrive_controller)
    ld.add_action(motion_control_node)
    ld.add_action(ekf_node)
    ld.add_action(static_tf_imu)
    ld.add_action(microros_agent_node)
    return ld
