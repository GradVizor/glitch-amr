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
    
