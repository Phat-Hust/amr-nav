"""Exercise the running C++ node with synthetic scans and a nontrivial TF chain."""

import math
import os
import subprocess
import time

import rclpy
from diagnostic_msgs.msg import DiagnosticArray, DiagnosticStatus
from geometry_msgs.msg import TransformStamped
from rclpy.qos import DurabilityPolicy, QoSProfile, qos_profile_sensor_data
from sensor_msgs.msg import LaserScan
from tf2_ros.static_transform_broadcaster import StaticTransformBroadcaster


def test_scan_notifications(tmp_path):
    rclpy.init()
    node = rclpy.create_node('scan_safety_test')
    notifications = []
    qos = QoSProfile(depth=1, durability=DurabilityPolicy.TRANSIENT_LOCAL)
    subscription = node.create_subscription(
        DiagnosticArray, '/scan_safety_test/diagnostics',
        lambda message: notifications.append(message.status[0]), qos)
    publisher = node.create_publisher(
        LaserScan, '/scan_safety_test/scan', qos_profile_sensor_data)
    broadcaster = StaticTransformBroadcaster(node)

    def transform(parent, child, x, yaw):
        result = TransformStamped()
        result.header.stamp = node.get_clock().now().to_msg()
        result.header.frame_id = parent
        result.child_frame_id = child
        result.transform.translation.x = x
        result.transform.rotation.z = math.sin(yaw / 2.)
        result.transform.rotation.w = math.cos(yaw / 2.)
        return result

    # A beam along laser +x becomes (0.2, range) in the footprint frame.
    broadcaster.sendTransform([
        transform('test_footprint', 'test_base', 0.1, 0.),
        transform('test_base', 'test_laser', 0.1, math.pi / 2.),
    ])
    log_path = tmp_path / 'node.log'
    with log_path.open('w') as log:
        process = subprocess.Popen([
            os.environ['SCAN_SAFETY_EXECUTABLE'], '--ros-args',
            '--params-file', os.environ['SCAN_SAFETY_CONFIG'],
            '-p', 'scan_topic:=/scan_safety_test/scan',
            '-p', 'diagnostics_topic:=/scan_safety_test/diagnostics',
            '-p', 'base_footprint_frame:=test_footprint',
            '-p', 'base_link_frame:=test_base',
            '-p', 'scan_timeout:=1.0',
        ], stdout=log, stderr=subprocess.STDOUT)

        def wait_for(level, text, ranges=None, frame='test_laser', stamp_age=0.):
            notifications.clear()
            deadline = time.monotonic() + 6.
            while time.monotonic() < deadline:
                assert process.poll() is None, log_path.read_text()
                if ranges is not None:
                    scan = LaserScan()
                    stamp = node.get_clock().now().nanoseconds - int(stamp_age * 1e9)
                    scan.header.stamp.sec, scan.header.stamp.nanosec = divmod(stamp, 10**9)
                    scan.header.frame_id = frame
                    scan.angle_min = 0.
                    scan.angle_max = max(0, len(ranges) - 1) * 0.01
                    scan.angle_increment = 0.01
                    scan.range_min = 0.05
                    scan.range_max = 10.
                    scan.ranges = ranges
                    publisher.publish(scan)
                rclpy.spin_once(node, timeout_sec=0.05)
                for status in notifications:
                    if status.level == level and text in status.message:
                        return status
            raise AssertionError(
                f'Missing {level}: {text}; received '
                f'{[(s.level, s.message) for s in notifications]}; {log_path.read_text()}')

        try:
            wait_for(DiagnosticStatus.ERROR, 'Waiting for a laser scan')
            wait_for(DiagnosticStatus.OK, 'clear', [1.0])
            warning = wait_for(DiagnosticStatus.WARN, 'warning rectangle', [0.35])
            assert {entry.key: entry.value for entry in warning.values}['warning_points'] == '1'
            error = wait_for(DiagnosticStatus.ERROR, 'error rectangle', [0.2])
            assert {entry.key: entry.value for entry in error.values}['error_points'] == '1'
            # Ignoring bad beams must not hide a valid obstacle.
            wait_for(DiagnosticStatus.ERROR, 'error rectangle', [float('nan'), 0.2])
            wait_for(DiagnosticStatus.OK, 'clear', [float('inf')])
            wait_for(DiagnosticStatus.ERROR, 'no usable measurements', [float('nan')])
            wait_for(DiagnosticStatus.ERROR, 'empty ranges', [])
            wait_for(DiagnosticStatus.ERROR, 'transform unavailable', [1.0], frame='missing_laser')
            wait_for(DiagnosticStatus.ERROR, 'timestamp', [1.0], stamp_age=2.)
            wait_for(DiagnosticStatus.OK, 'clear', [1.0])
            wait_for(DiagnosticStatus.ERROR, 'timeout')
            wait_for(DiagnosticStatus.OK, 'clear', [1.0])
        finally:
            process.terminate()
            try:
                process.wait(timeout=5.)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=5.)
            node.destroy_subscription(subscription)
            node.destroy_node()
            rclpy.shutdown()
