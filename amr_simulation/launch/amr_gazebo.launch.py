#!/usr/bin/env python3

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, AppendEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():

    AppendEnvironmentVariable(
        'IGN_GAZEBO_RESOURCE_PATH',
        os.path.expanduser('~/.gazebo/models')
    )


    pkg_amr_simulation = get_package_share_directory('amr_simulation')
    pkg_ros_gz_sim = get_package_share_directory('ros_gz_sim')

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')
    x_pose = LaunchConfiguration('x_pose', default='1.0')
    y_pose = LaunchConfiguration('y_pose', default='0.0')
    world_name = LaunchConfiguration('world_name', default='old_office')
    robot_name = LaunchConfiguration('robot_name', default='mecanum')
    prefix = LaunchConfiguration('prefix', default='')

    world_path = PathJoinSubstitution([
        FindPackageShare('amr_simulation'),
        'worlds',
        [world_name, '.world']
    ])

    # 1. Launch Ignition Gazebo (gz_sim) - replaces gzserver & gzclient
    # '-r' runs simulation immediately on start
    gz_sim_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_ros_gz_sim, 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': ['-r ', world_path]}.items()
    )

    # 2. Bridge Gazebo /clock to ROS 2 /clock
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='gz_bridge',
        output='screen',
        arguments=[
            '/clock@rosgraph_msgs/msg/Clock[ignition.msgs.Clock'
        ],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    # 3. Call robot_state_publisher.launch.py
    robot_state_publisher_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_amr_simulation, 'launch', 'robot_state_publisher.launch.py')
        ),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'robot_name': robot_name,
            'prefix': prefix,
            'use_gazebo': 'true',
            'use_rviz': 'false',
            'use_jsp': 'false',
            'jsp_gui': 'false',
            'enable_odom_tf': 'true'
        }.items()
    )

    # 4. Spawn Robot using spawn_amr_robot.launch.py
    spawn_amr_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_amr_simulation, 'launch', 'spawn_amr_robot.launch.py')
        ),
        launch_arguments={
            'x_pose': x_pose,
            'y_pose': y_pose,
            'robot_name': robot_name,
        }.items()
    )

    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='true',
                              description='Use simulation clock'),
        DeclareLaunchArgument('x_pose', default_value='1.0',
                              description='Spawn X position'),
        DeclareLaunchArgument('y_pose', default_value='0.0',
                              description='Spawn Y position'),
        DeclareLaunchArgument('world_name', default_value='warehouse',
                              description='World file name (without .world)'),
        DeclareLaunchArgument('robot_name', default_value='mecanum',
                              description='Name of the robot'),
        DeclareLaunchArgument('prefix', default_value='',
                              description='Joint/link prefix'),

        gz_sim_cmd,
        bridge_node,
        robot_state_publisher_cmd,
        spawn_amr_cmd
    ])