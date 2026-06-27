# Copyright 2023 Clearpath Robotics, Inc.
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
# @author Roni Kreinin (rkreinin@clearpathrobotics.com)
# Modified by Reishabh Rathore <reishabhrathore2003@gmail.com> (2026)

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import launch_configuration
from launch_ros.actions import Node

ARGUMENTS = [
    DeclareLaunchArgument('use_sim_time', default_value='true',
                          choices=['true', 'false'],
                          description='Use sim time'),
    DeclareLaunchArgument('robot_name', default_value='glitch',
                          description='Ignition model name'),
    DeclareLaunchArgument('world', default_value='warehouse',
                          description='World name'),
]


def generate_launch_description():
    use_sim_time = LaunchConfiguration('use_sim_time')
    robot_name = LaunchConfiguration('robot_name')
    world = LaunchConfiguration('world')

    # lidar bridge
    lidar_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='lidar_bridge',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time
        }],
        arguments=[
            ['/world/', world,
             '/model/', robot_name,
             '/link/rplidar_link/sensor/rplidar/scan' +
             '@sensor_msgs/msg/LaserScan[ignition.msgs.LaserScan']
        ],
        remappings=[
            (['/world/', world,
              '/model/', robot_name,
              '/link/rplidar_link/sensor/rplidar/scan'],
             'scan')
        ])
    
    # odom to base_link transform bridge
    odom_base_tf_bridge = Node(package='ros_gz_bridge', executable='parameter_bridge',
                               name='odom_base_tf_bridge',
                               output='screen',
                               parameters=[{
                                   'use_sim_time': use_sim_time
                               }],
                               arguments=[
                                   ['/model/', robot_name, '/tf' +
                                    '@tf2_msgs/msg/TFMessage' +
                                    '[gz.msgs.Pose_V']
                               ],
                               remappings=[
                                   (['/model/', robot_name, '/tf'], 'tf')
                               ])

    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    ld.add_action(lidar_bridge)
    ld.add_action(odom_base_tf_bridge)
    return ld
