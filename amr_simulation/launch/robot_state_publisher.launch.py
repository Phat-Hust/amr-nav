#!/usr/bin/env python3

import os
from ament_index_python.packages import get_package_prefix
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.conditions import IfCondition
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def process_ros2_controllers_config(context):
    prefix = LaunchConfiguration('prefix').perform(context)
    enable_odom_tf = LaunchConfiguration('enable_odom_tf').perform(context)

    # Dynamically locate source directory from install prefix
    ws_dir = os.path.abspath(os.path.join(get_package_prefix('amr_simulation'), '..', '..'))
    src_config_dir = os.path.join(ws_dir, 'src', 'amr_simulation', 'config', 'mecanum')
    template_path = os.path.join(src_config_dir, 'ros2_controllers_template.yaml')

    if os.path.exists(template_path):
        with open(template_path, 'r', encoding='utf-8') as file:
            template_content = file.read()

        processed_content = template_content.replace('${prefix}', prefix)
        processed_content = processed_content.replace(
            'enable_odom_tf: true', f'enable_odom_tf: {enable_odom_tf}'
        )

        os.makedirs(src_config_dir, exist_ok=True)
        output_path = os.path.join(src_config_dir, 'ros2_controllers.yaml')
        with open(output_path, 'w', encoding='utf-8') as file:
            file.write(processed_content)

    return []


ARGUMENTS = [
    DeclareLaunchArgument('robot_name', default_value='mecanum',
                          description='Name of the robot'),
    DeclareLaunchArgument('prefix', default_value='',
                          description='Prefix for robot joints and links'),
    DeclareLaunchArgument('use_gazebo', default_value='false',
                          choices=['true', 'false'],
                          description='Whether to use Gazebo simulation'),
    DeclareLaunchArgument('enable_odom_tf', default_value='true',
                          choices=['true', 'false'],
                          description='Enable odometry transform broadcasting via ROS 2 Control'),
    DeclareLaunchArgument('jsp_gui', default_value='false',
                          choices=['true', 'false'],
                          description='Flag to enable joint_state_publisher_gui'),
    DeclareLaunchArgument('use_jsp', default_value='false',
                          choices=['true', 'false'],
                          description='Enable the joint state publisher'),
    DeclareLaunchArgument('use_rviz', default_value='false',
                          choices=['true', 'false'],
                          description='Whether to start RVIZ'),
    DeclareLaunchArgument('use_sim_time', default_value='false',
                          choices=['true', 'false'],
                          description='Use simulation (Gazebo) clock if true')
]


def generate_launch_description():
    urdf_package = 'amr_simulation'
    urdf_filename = 'mecanum_drive.urdf.xacro'
    rviz_config_filename = 'mecanum.rviz'

    pkg_share_description = FindPackageShare(urdf_package)

    default_urdf_model_path = PathJoinSubstitution(
        [pkg_share_description, 'urdf', 'robots', urdf_filename])
    default_rviz_config_path = PathJoinSubstitution(
        [pkg_share_description, 'rviz', rviz_config_filename])

    jsp_gui = LaunchConfiguration('jsp_gui')
    rviz_config_file = LaunchConfiguration('rviz_config_file')
    urdf_model = LaunchConfiguration('urdf_model')
    use_jsp = LaunchConfiguration('use_jsp')
    use_rviz = LaunchConfiguration('use_rviz')
    use_sim_time = LaunchConfiguration('use_sim_time')

    declare_rviz_config_file_cmd = DeclareLaunchArgument(
        name='rviz_config_file',
        default_value=default_rviz_config_path,
        description='Full path to the RVIZ config file to use')

    declare_urdf_model_path_cmd = DeclareLaunchArgument(
        name='urdf_model',
        default_value=default_urdf_model_path,
        description='Absolute path to robot urdf/xacro file')

    # Parse XACRO/URDF dynamically with arguments
    robot_description_content = ParameterValue(Command([
        'xacro', ' ', urdf_model, ' ',
        'robot_name:=', LaunchConfiguration('robot_name'), ' ',
        'prefix:=', LaunchConfiguration('prefix'), ' ',
        'use_gazebo:=', LaunchConfiguration('use_gazebo')
    ]), value_type=str)

    start_robot_state_publisher_cmd = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time,
            'robot_description': robot_description_content
        }]
    )

    start_joint_state_publisher_cmd = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        name='joint_state_publisher',
        parameters=[{'use_sim_time': use_sim_time}],
        condition=IfCondition(use_jsp)
    )

    start_joint_state_publisher_gui_cmd = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        parameters=[{'use_sim_time': use_sim_time}],
        condition=IfCondition(jsp_gui)
    )

    start_rviz_cmd = Node(
        condition=IfCondition(use_rviz),
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    ld = LaunchDescription(ARGUMENTS)

    ld.add_action(OpaqueFunction(function=process_ros2_controllers_config))
    ld.add_action(declare_rviz_config_file_cmd)
    ld.add_action(declare_urdf_model_path_cmd)

    ld.add_action(start_robot_state_publisher_cmd)
    ld.add_action(start_joint_state_publisher_cmd)
    ld.add_action(start_joint_state_publisher_gui_cmd)
    ld.add_action(start_rviz_cmd)

    return ld