# scan_safety

ROS 2 node that reports obstacles in two rectangular zones around the AMR:

- `OK` (0): neither zone reaches `min_points` scan hits.
- `WARN` (1): the outer warning zone reaches `min_points` hits.
- `ERROR` (2): the inner error zone reaches `min_points` hits, or scan data/TF is unavailable.

Notifications use `diagnostic_msgs/msg/DiagnosticArray` on
`/scan_safety/diagnostics` by default. Status changes are also logged to the console.
The node only reports status; it does not command the motors or stop the robot.

## Configuration

The source template `config/config.yaml` installs as
`/var/amr/config/scan_safety.yaml`. The launch file reads
`$CONFIG_FOLDER/scan_safety.yaml`, defaulting to `/var/amr/config/scan_safety.yaml`.
Source `/var/amr/config/amr_config.sh` before launching. Edit the installed YAML
for robot-specific settings, or pass a custom file at launch.
It defines the scan and diagnostics topics, `base_footprint_frame`,
`base_link_frame`, timeout values, hit threshold, and four corners of each zone.

Each `points` parameter is a flat list of eight floating-point values:
`[x1, y1, x2, y2, x3, y3, x4, y4]`. ROS 2 parameters do not support nested arrays.
Coordinates are meters in `base_footprint_frame`, with +x forward and +y left.
List corners in perimeter order. Clockwise, counterclockwise, and rotated
rectangles are supported, including points on the boundary. The error rectangle
must fit inside the warning rectangle. Invalid configuration stops node startup.
The supplied coordinates are examples to adjust for your robot.

The scan's `header.frame_id` supplies the LiDAR frame. At each scan timestamp,
the node transforms scan points through `base_link_frame` into
`base_footprint_frame`, then checks their XY positions. The corresponding TF
chain must be available. All beams use the scan header timestamp; the node
does not compensate for robot motion during a scan.

The node starts in ERROR until it receives a usable scan. A wall-clock watchdog
reports ERROR when scans stop, including when simulation time is paused. Stale
timestamps, missing transforms, malformed scans, and scans containing only
invalid ranges also produce ERROR. Positive infinity is treated as no return;
NaN, negative infinity, and out-of-range finite measurements are ignored.
Warning counts include hits inside the error rectangle. ERROR takes priority,
and a fresh clear scan restores OK. Parameters are read at startup; restart
the node after changing configuration.

## Build and run

From the workspace root:

```bash
colcon build --packages-select scan_safety
source install/setup.bash
ros2 launch scan_safety scan_safety.launch.py
```

For a custom configuration or simulation:

```bash
ros2 launch scan_safety scan_safety.launch.py params_file:=/path/to/config.yaml use_sim_time:=true
```

Monitor notifications:

```bash
ros2 topic echo /scan_safety/diagnostics
```
