import os

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, LogInfo
from launch.launch_description_sources import PythonLaunchDescriptionSource

from ament_index_python.packages import get_package_share_directory


def generate_launch_description():

    # ---------------------------------------------------------
    # Read LiDAR model from environment variable
    #
    # Example:
    #   export AMR_LIDAR_MODEL=a2m8
    # ---------------------------------------------------------
    lidar_model = os.getenv("AMR_LIDAR_MODEL", "a2m8")
    lidar_model = lidar_model.strip().lower()

    # ---------------------------------------------------------
    # Find rplidar_ros package
    # ---------------------------------------------------------
    rplidar_share = get_package_share_directory("rplidar_ros")

    # Convert:
    #   a2m8 -> rplidar_a2m8_launch.py
    #   s2   -> rplidar_s2_launch.py
    #   a1   -> rplidar_a1_launch.py
    # ---------------------------------------------------------
    launch_filename = f"rplidar_{lidar_model}_launch.py"

    launch_path = os.path.join( rplidar_share, "launch",launch_filename)

    # ---------------------------------------------------------
    # Validate selected model
    # ---------------------------------------------------------
    if not os.path.isfile(launch_path):
        launch_directory = os.path.join(
            rplidar_share,
            "launch"
        )

        available_models = []

        if os.path.isdir(launch_directory):
            for filename in os.listdir(launch_directory):
                if (
                    filename.startswith("rplidar_") and filename.endswith("_launch.py")
                ):
                    model = filename[
                        len("rplidar_"):-len("_launch.py")
                    ]

                    available_models.append(model)

        raise RuntimeError(
            "\n"
            f"[AMR LiDAR] Unsupported LiDAR model: {lidar_model}\n"
            f"[AMR LiDAR] Available models: "
            f"{', '.join(sorted(available_models))}\n"
        )

    # ---------------------------------------------------------
    # Start selected RPLIDAR launch
    # ---------------------------------------------------------
    rplidar_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            launch_path
        )
    )

    return LaunchDescription([
        LogInfo(
            msg=f"[AMR LiDAR] Selected model: {lidar_model}"
        ),

        LogInfo(
            msg=f"[AMR LiDAR] Launch file: {launch_filename}"
        ),

        rplidar_launch
    ])