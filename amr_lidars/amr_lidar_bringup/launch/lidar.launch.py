"""Select and include one LiDAR driver's launch file."""

import os
from pathlib import Path

from ament_index_python.packages import get_package_share_directory, PackageNotFoundError
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, LogInfo, OpaqueFunction
from launch.launch_description_sources import AnyLaunchDescriptionSource
from launch.substitutions import EnvironmentVariable, LaunchConfiguration
import yaml


def launch_selected_lidar(context):
    lidar_type = LaunchConfiguration('lidar_type').perform(context).strip().lower()
    config_file = LaunchConfiguration('config_file').perform(context)
    config_folder = os.environ.get('CONFIG_FOLDER', '/var/amr/config')

    try:
        with open(config_file, encoding='utf-8') as stream:
            configuration = yaml.safe_load(stream)
    except (OSError, yaml.YAMLError) as error:
        raise RuntimeError(
            f'[AMR LiDAR] Cannot read configuration {config_file}: {error}') from error

    drivers = configuration.get('drivers') if isinstance(configuration, dict) else None
    if not isinstance(drivers, dict) or not drivers:
        raise RuntimeError(f'[AMR LiDAR] {config_file} must contain a nonempty "drivers" mapping')
    if lidar_type not in drivers:
        supported = ', '.join(sorted(str(model) for model in drivers))
        raise RuntimeError(
            f'[AMR LiDAR] Unsupported LIDAR_TYPE {lidar_type!r}. Available models: {supported}')

    driver = drivers[lidar_type]
    if not isinstance(driver, dict):
        raise RuntimeError(f'[AMR LiDAR] Driver entry for {lidar_type} must be a mapping')
    package = driver.get('package')
    launch_file = driver.get('launch_file')
    if not isinstance(package, str) or not package.strip():
        raise RuntimeError(f'[AMR LiDAR] {lidar_type} requires a nonempty "package"')
    if not isinstance(launch_file, str) or not launch_file.strip():
        raise RuntimeError(f'[AMR LiDAR] {lidar_type} requires a nonempty "launch_file"')

    arguments = driver.get('launch_arguments', {})
    if not isinstance(arguments, dict):
        raise RuntimeError(f'[AMR LiDAR] {lidar_type}.launch_arguments must be a mapping')
    launch_arguments = {}
    for name, value in arguments.items():
        if not isinstance(name, str) or not isinstance(value, (str, bool, int, float)):
            raise RuntimeError(
                '[AMR LiDAR] Launch argument names must be strings and values scalars')
        text = str(value).lower() if isinstance(value, bool) else str(value)
        launch_arguments[name] = text.replace('${CONFIG_FOLDER}', config_folder)

    try:
        package_share = get_package_share_directory(package)
    except PackageNotFoundError as error:
        raise RuntimeError(
            f'[AMR LiDAR] {lidar_type} requires package {package!r}, which is not installed. '
            'Add the driver source, build it, and source install/setup.bash before launching.'
        ) from error

    launch_path = Path(package_share) / launch_file.replace('${CONFIG_FOLDER}', config_folder)
    if not launch_path.is_file():
        raise RuntimeError(
            f'[AMR LiDAR] Launch file for {lidar_type} does not exist: {launch_path}. '
            f'Check the package and launch_file entry in {config_file}.')

    return [
        LogInfo(msg=f'[AMR LiDAR] Selected {lidar_type}: {launch_path}'),
        IncludeLaunchDescription(
            AnyLaunchDescriptionSource(str(launch_path)),
            launch_arguments=launch_arguments.items(),
        ),
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'lidar_type',
            default_value=EnvironmentVariable('LIDAR_TYPE', default_value='a2m8'),
            description='LiDAR model key from amr_lidars.yaml; defaults to LIDAR_TYPE'),
        DeclareLaunchArgument(
            'config_file',
            default_value=[
                EnvironmentVariable('CONFIG_FOLDER', default_value='/var/amr/config'),
                '/amr_lidars.yaml',
            ],
            description='LiDAR driver launch registry'),
        OpaqueFunction(function=launch_selected_lidar),
    ])
