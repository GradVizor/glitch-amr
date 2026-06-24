from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, GroupAction)
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node, PushRosNamespace


ARGUMENTS = [
    DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation (Gazebo) clock if true'),
    DeclareLaunchArgument(
        'namespace',
        default_value='',
        description='Robot namespace'
    ),
]


def generate_launch_description():

    pkg_glitch_navigation = get_package_share_directory('glitch_navigation')

    nav2_config = PathJoinSubstitution(
        [pkg_glitch_navigation, 'config', 'navigation.rviz'])

    namespace = LaunchConfiguration('namespace')

    rviz = GroupAction([
        PushRosNamespace(namespace),

        Node(package='rviz2',
             executable='rviz2',
             name='rviz2',
             arguments=['-d', nav2_config],
             parameters=[{'use_sim_time': LaunchConfiguration('use_sim_time')}],
             remappings=[
                ('/tf', 'tf'),
                ('/tf_static', 'tf_static')
             ],
             output='screen'),
    ])

    ld = LaunchDescription(ARGUMENTS)
    ld.add_action(rviz)
    return ld
