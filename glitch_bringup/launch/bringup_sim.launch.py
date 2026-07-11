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
    

    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    # Include robot description
    ld.add_action(diffdrive_controller)
    ld.add_action(motion_control_node)
    return ld
