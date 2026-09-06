#!/usr/bin/env python3

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    x_pose = LaunchConfiguration('x_pose')
    y_pose = LaunchConfiguration('y_pose')
    z_pose = LaunchConfiguration('z_pose')
    robot_name = LaunchConfiguration('robot_name')

    declare_x_position_cmd = DeclareLaunchArgument(
        'x_pose', default_value='0.0',
        description='Initial x position of the robot'
    )
    declare_y_position_cmd = DeclareLaunchArgument(
        'y_pose', default_value='0.0',
        description='Initial y position of the robot'
    )
    declare_z_position_cmd = DeclareLaunchArgument(
        'z_pose', default_value='0.05',
        description='Initial z position of the robot'
    )
    declare_robot_name_cmd = DeclareLaunchArgument(
        'robot_name', default_value='mecanum',
        description='Entity name in Gazebo'
    )

    # Spawn entity using ros_gz_sim
    spawn_cmd = Node(
        package='ros_gz_sim',
        executable='create',
        output='screen',
        arguments=[
            '-name', robot_name,
            '-topic', 'robot_description',
            '-x', x_pose,
            '-y', y_pose,
            '-z', z_pose
        ]
    )

    return LaunchDescription([
        declare_x_position_cmd,
        declare_y_position_cmd,
        declare_z_position_cmd,
        declare_robot_name_cmd,
        spawn_cmd,
    ])