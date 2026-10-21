#!/usr/env/bin python3

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

# get environment variable ==> get LIDARS => a1c8
def generate_launch_description():

	ld = LaunchDescription();

	config_directory = os.environ.get('CONFIG_FOLDER', '/var/amr/config')
	default_config = os.path.join(config_directory, 'pi_amr_controller.yaml')

	params_file = LaunchConfiguration('params_file');

	params_file_cmd = DeclareLaunchArgument(
		'params_file',
		default_value= default_config
	)

	# Using environment variable to config type of lidars in sh file

	# amr_lidars = IncludeLaunchDescription(
	# 	PythonLaunchDescriptionSource(
	# 		os.path.join('amr_lidars'), 'launch', 'amr_lidar.launch.py',
	# 		launch_arguments={}
	# 	)
	# )

	node = Node(
		package='pi_amr_controller',
		name='app_controller_node', 
		executable='pi_amr_controller',
		parameters=[params_file]
	)

	ld.add_action(params_file_cmd)
	ld.add_action(node)

	return ld


