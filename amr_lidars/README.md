# AMR LiDAR selection

`amr_lidar_bringup` reads `LIDAR_TYPE` from the environment and includes exactly
one driver launch file. The driver registry is installed at
`/var/amr/config/amr_lidars.yaml`; `CONFIG_FOLDER` selects a different directory.

Set one of these model keys in `/var/amr/config/amr_config.sh`:

| `LIDAR_TYPE` | Driver package | Launch file |
| --- | --- | --- |
| `ydlidar_x4` | `ydlidar_ros2_driver` | `launch/ydlidar_launch.py` |
| `sick_tim571` | `sick_scan_xd` | `launch/sick_tim_5xx.launch.py` |
| `a2m8` | `rplidar_ros` | `launch/rplidar_a2m8_launch.py` |
| `a1` | `rplidar_ros` | `launch/rplidar_a1_launch.py` |
| `s2` | `rplidar_ros` | `launch/rplidar_s2_launch.py` |

The new entries target the upstream
[YDLIDAR ROS 2 launch](https://github.com/YDLIDAR/ydlidar_ros2_driver/blob/humble/launch/ydlidar_launch.py)
and [SICK TiM5xx launch](https://github.com/SICKAG/sick_scan_xd/blob/develop/launch/sick_tim_5xx.launch.py).
Adjust the registry to match the driver sources and versions added later.
These sources are not included or downloaded by this package. Only the selected
driver must be installed; missing packages and launch files produce clear errors.

From the workspace root, build and source the workspace, then launch:

```bash
colcon build --packages-select amr_lidar_bringup
source install/setup.bash
source /var/amr/config/amr_config.sh
ros2 launch amr_lidar_bringup lidar.launch.py
```

For a temporary selection without editing the environment file:

```bash
ros2 launch amr_lidar_bringup lidar.launch.py lidar_type:=ydlidar_x4
ros2 launch amr_lidar_bringup lidar.launch.py lidar_type:=sick_tim571
```

The `config_file` launch argument can override the registry path. Each driver
entry defines a package name, a launch path relative to its package share
directory, and optional `launch_arguments`. `${CONFIG_FOLDER}` in launch paths
or argument values expands to the shared configuration directory. An absolute
launch path can select a custom wrapper in that directory.

When adding the YDLIDAR source, supply `/var/amr/config/ydlidar_x4.yaml` with its
X4 driver settings; the selector forwards that path as `params_file`. The
upstream SICK Python wrapper reads arguments from the process command line,
so generic include arguments do not configure that wrapper. Its network/device
settings and any wrapper needed to read central configuration will be integrated
when the SICK source is added. Current SICK selection invokes its stock launch
with driver defaults. Confirm each driver's scan topic and sensor TF against
the robot description when integrating the hardware.

No driver is required for `--show-args` or for the selector unit tests.
