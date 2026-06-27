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
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution


def generate_launch_description():
    pkg_nav2_bringup = get_package_share_directory('nav2_bringup')
    pkg_glitch_navigation = get_package_share_directory('glitch_navigation')
    
    nav2_params = PathJoinSubstitution(
        [pkg_glitch_navigation, 'config', 'nav2_params.yaml'])
    
    map_file = PathJoinSubstitution(
        [pkg_glitch_navigation, 'maps', 'warehouse.yaml'])
    
    nav2_bringup = PathJoinSubstitution(
        [pkg_nav2_bringup, 'launch', 'bringup_launch.py'])
    

    nav2 = LaunchDescription([
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([nav2_bringup]),
            launch_arguments={
            'map':map_file,
            'params_file': nav2_params,
            'use_sim_time': 'True'}.items()
        )
    ])
    ld = LaunchDescription()
    ld.add_action(nav2)
    return ld
    
