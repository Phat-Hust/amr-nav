import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.substitutions import TextSubstitution
from launch.actions import (
    OpaqueFunction,
    DeclareLaunchArgument,
    IncludeLaunchDescription,
)

def launch_setup(context, *args, **kwargs):

    use_sim_time_str = LaunchConfiguration('use_sim_time').perform(context).strip().lower()

    use_sim_time = use_sim_time_str in ('true', '1')

    if not use_sim_time:
        urdf_path = '/var/amr/config/amr_1lidar.urdf'
    else:


    urdf_file_name = LaunchConfiguration('urdf_file_name', default='amr.urdf')

    urdf_path = urdf_file_name.perform(context=context)

    with open(urdf_path, 'r') as infp:
        robot_desc = infp.read()

    return [
        Node (
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters= [{
                'use_sim_time': use_sim_time,
                'robot_description': robot_desc
            }],
        ),
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'),
        DeclareLaunchArgument(
            'urdf_file_name',
            default_value='amr.urdf',
            description='Urdf file to be used'
        ),

        OpaqueFunction(function=launch_setup)
    ])

