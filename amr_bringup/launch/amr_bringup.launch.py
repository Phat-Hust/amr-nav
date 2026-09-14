#!/usr/bin/python3

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, SetEnvironmentVariable, IncludeLaunchDescription, Shutdown
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource, FrontendLaunchDescriptionSource
from nav2_common.launch import RewrittenYaml
from launch.conditions import IfCondition, UnlessCondition

CONFIG_FOLDER = os.getenv('CONFIG_FOLDER', '/var/amr/config')
NUMBER_OF_SCANS = os.getenv('NUMBER_OF_SCANS', '1')
PLUGINS = os.getenv('PLUGINS', '').split()


if int(NUMBER_OF_SCANS) < 1 and int(NUMBER_OF_SCANS) > 2:
    print(f'[ERROR] Number of LIDAR = {NUMBER_OF_SCANS} not supported')
    exit()

def is_package_installed(package_name):
   try:
      get_package_share_directory(package_name)
      return True
   except:
      return False

def generate_launch_description():
	use_sim_time = LaunchConfiguration('use_sim_time')
    simulation_mode = LaunchConfiguration('simulation_mode')
    run_gazebo_client = LaunchConfiguration('run_gazebo_client')
    run_rviz = LaunchConfiguration('run_rviz')
    params_file = LaunchConfiguration('params_file')

    declare_use_sim_time_cmd = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation (Gazebo) clock if true')
    simulation_mode_arg = DeclareLaunchArgument('simulation_mode',
          default_value='false',
          description='Launch simulation mode')
    run_gazebo_client_arg = DeclareLaunchArgument('run_gazebo_client',
          default_value='false',
          description='Launch headless mode')
    run_rviz_arg = DeclareLaunchArgument('run_rviz',
          default_value='false')
    param_file_arg = DeclareLaunchArgument('params_file',
          default_value=os.path.join(CONFIG_FOLDER, 'tvc_bringup_params.yaml'),
          description='')

    param_substitutions = {
	    'use_sim_time': use_sim_time,
	    'amr_loc_config_directory': CONFIG_FOLDER,
	    'configuration_directory': CONFIG_FOLDER,
	    'number_of_scans': NUMBER_OF_SCANS
	}

	configured_params = RewrittenYaml(
	    source_file=params_file,
	    root_key='',
	    param_rewrites=param_substitutions,
	    convert_types=True)

	# --------- ROBOT INFORMATION -------------------------------------------
	simulation_nodes = GroupAction(
	    condition=IfCondition(simulation_mode),
	    actions=[
	      # Simulation
	      # The robot state publisher is run inside this launch file
	      IncludeLaunchDescription(
	        PythonLaunchDescriptionSource(os.path.join(get_package_share_directory('amr_simulation'), 'launch', 'amr_gazebo.launch.py')),
	        launch_arguments={'world_name':LaunchConfiguration('world_name', default='old_office'),
	                          'run_gazebo_client':run_gazebo_client}.items(),
	      ),
	      IncludeLaunchDescription(
	        PythonLaunchDescriptionSource(os.path.join(get_package_share_directory('amr_simulation'), 'launch', 'robot_state_publisher.launch.py')),
	        launch_arguments={'use_sim_time': 'false',
	                          'urdf_file_name': os.path.join(CONFIG_FOLDER, 'amr_' + NUMBER_OF_SCANS + 'lidar.urdf')}.items()
	      ),
	    ]
	  )
	robot_hw_group = GroupAction(
	    condition=UnlessCondition(simulation_mode),
	    actions=[
	      # Motor Driver
	      # IncludeLaunchDescription(
	      #   PythonLaunchDescriptionSource(os.path.join(get_package_share_directory('pi_amr_controller'),'launch', 'pi_amr_controller.launch.py')),
	      #   launch_arguments={'params_file': os.path.join(CONFIG_FOLDER, "pi_amr_controller.yaml"),
	      #                     'use_sim_time': use_sim_time}.items()
	      # ),
	      # Lidar
	      IncludeLaunchDescription(
	        PythonLaunchDescriptionSource(os.path.join(get_package_share_directory('amr_lidar'),'launch', 'amr_lidar.launch.py'))
	      ),
	      IncludeLaunchDescription(
	        PythonLaunchDescriptionSource(os.path.join(get_package_share_directory('amr_description'), 'launch', 'robot_state_publisher.launch.py')),
	        launch_arguments={'use_sim_time': 'false',
	                          'urdf_file_name': os.path.join(CONFIG_FOLDER, 'amr_' + NUMBER_OF_SCANS + 'lidar.urdf')}.items()
	      ),
	    ]
	  )

	rviz_node = GroupAction(
	    condition=IfCondition(run_rviz),
	    actions = [
	      IncludeLaunchDescription(
	        PythonLaunchDescriptionSource(
	            os.path.join(get_package_share_directory('nav2_bringup'), 'launch', 'rviz_launch.py')),
	        launch_arguments={'namespace': '',
	                          'use_namespace': 'False',
	                          'rviz_config': os.path.join(get_package_share_directory('amr_app_manager'), 'rviz','mecanum_amr.rviz')}.items())
	    ]
	  )

	ld = LaunchDescription()
    ld.add_action(declare_use_sim_time_cmd)
    ld.add_action(simulation_mode_arg)
    # ld.add_action(sim_world_name)
    ld.add_action(run_gazebo_client_arg)
    ld.add_action(run_rviz_arg)
    ld.add_action(param_file_arg)
  
    ld.add_action(simulation_nodes)
    ld.add_action(robot_hw_group)

    ld.add_action(rviz_node)

    return ld















