#ifndef APP_CONTROLLER_HPP_
#define APP_CONTROLLER_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
// ROS2
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "tf2_ros/transform_broadcaster.h"

class AppController : public rclcpp::Node
{
public:
  explicit AppController(
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions()
  );

  ~AppController();

private:
  /* =========================
     SERIAL
     ========================= */

  void init_serial();

  bool read_hardware_velocities(
    double &v1,
    double &v2,
    double &v3,
    double &v4
  );

  /* =========================
     ROS2
     ========================= */

  void update_loop();

  void cmd_vel_callback(
    const geometry_msgs::msg::Twist::SharedPtr msg
  );

  /* =========================
     MECANUM KINEMATICS
     ========================= */

  void compute_wheel_targets(
    double vx,
    double vy,
    double wz,
    double &w1,
    double &w2,
    double &w3,
    double &w4
  );

  void send_wheel_targets(
    double w1,
    double w2,
    double w3,
    double w4
  );

  /* =========================
     BINARY COMMUNICATION

     Frame 11 bytes:

     [0xAA][0x55]
     [v1_L][v1_H]
     [v2_L][v2_H]
     [v3_L][v3_H]
     [v4_L][v4_H]
     [CRC]

     v1...v4:
     int16_t, đơn vị mm/s
     ========================= */

  uint8_t calculate_crc(
    const uint8_t *data,
    std::size_t length
  );

  int16_t velocity_to_int16(
    double velocity_m_s
  );

  bool send_hardware_velocities(
    double v1,
    double v2,
    double v3,
    double v4
  );

  /* =========================
     SERIAL STATE
     ========================= */

  std::string port_name_{"/dev/ttyACM1"};
  int serial_fd_{-1};

  std::vector<uint8_t> serial_rx_buffer_;


  /* =========================
     ROBOT PARAMETERS
     ========================= */

  // Bán kính bánh: đường kính 97 mm
  double r_{0.0485};

  // Khoảng cách từ tâm robot tới bánh
  double lx_{0.25};
  double ly_{0.25};

  /*
    Giới hạn tốc độ dài của từng bánh.

    Giá trị này phải được chỉnh theo
    tốc độ thực tế của motor.
  */
  double max_wheel_speed_mps_{2.0};

  /* =========================
     VELOCITY COMPENSATION
     ========================= */

  double longitudinal_gain_{1.0};
  double lateral_gain_{1.0};
  double angular_gain_{1.0};

  /* =========================
     CMD_VEL STATE
     ========================= */

  double target_vx_{0.0};
  double target_vy_{0.0};
  double target_wz_{0.0};

  /*
    Nếu không nhận /cmd_vel trong thời gian này,
    node tự đặt vận tốc về 0.
  */
  double cmd_vel_timeout_s_{0.5};

  rclcpp::Time last_cmd_vel_time_;

  /* =========================
     ODOMETRY STATE
     ========================= */

  double x_{0.0};
  double y_{0.0};
  double theta_{0.0};

  rclcpp::Time last_time_;

  /* =========================
     ROS2 OBJECTS
     ========================= */

  rclcpp::TimerBase::SharedPtr timer_;

  rclcpp::Publisher<
    nav_msgs::msg::Odometry
  >::SharedPtr odom_pub_;

  rclcpp::Subscription<
    geometry_msgs::msg::Twist
  >::SharedPtr cmd_vel_sub_;

  std::unique_ptr<
    tf2_ros::TransformBroadcaster
  > tf_broadcaster_;
};

#endif  // APP_CONTROLLER_HPP_