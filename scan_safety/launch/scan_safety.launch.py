import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    config_directory = os.environ.get('CONFIG_FOLDER', '/var/amr/config')
    default_config = os.path.join(config_directory, 'scan_safety.yaml')
    return LaunchDescription([
        DeclareLaunchArgument('params_file', default_value=str(default_config)),
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        Node(
            package='scan_safety',
            executable='scan_safety_node',
            name='scan_safety',
            output='screen',
            parameters=[
                LaunchConfiguration('params_file'),
                {'use_sim_time': ParameterValue(
                    LaunchConfiguration('use_sim_time'), value_type=bool)},
            ],
        ),
    ])
