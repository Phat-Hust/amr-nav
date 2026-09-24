#ifndef PI_AMR_CONTROLLER_HPP_
#define PI_AMR_CONTROLLER_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cmath>
#include <cstring>

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>

#include <nav_msgs/msg/odometry.hpp>

#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>

// Custom message
#include "amr_common/msg/wheel_telemetry.hpp"

class PiAmrController : public rclcpp::Node
{
public:
    explicit PiAmrController(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~PiAmrController();

private:
    void initSerial();
    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void pidCallback(const geometry_msgs::msg::Vector3::SharedPtr msg);
    
    void sendTargetVelocity(const double &vx, const double &vy, const double &wz);
    void sendPidParameters(double kp, double ki, double kd);
    void processSerialData(); // Hàm bóc tách dữ liệu linh hoạt (vận tốc & PID)
    
    void updateLoop();
    uint8_t calculateCRC(const uint8_t* data, std::size_t length);
    int16_t velocityToInt16(double velocity_m_s);

    // Serial parameters
    int serial_fd_{-1};
    std::string port_name_;
    std::vector<uint8_t> serial_rx_buffer_;

    // Robot parameters
    double radius_{0.05};
    double base_x_{0.2};
    double base_y_{0.2};

    // Speeds & Pose
    double target_vx_{0.0}, target_vy_{0.0}, target_wz_{0.0};
    double target_v_[4]{0.0, 0.0, 0.0, 0.0};
    double current_v1_{0.0}, current_v2_{0.0}, current_v3_{0.0}, current_v4_{0.0};
    double current_kp_{0.0}, current_ki_{0.0}, current_kd_{0.0};

    // Odometry state
    double x_{0.0}, y_{0.0}, yaw_{0.0};
    rclcpp::Time last_time_;

    // ROS 2 communications
    
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    rclcpp::Publisher<amr_common::msg::WheelTelemetry>::SharedPtr wheel_telemetry_pub_; // Unified topic
    // rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr get_pid_pub_;

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr pid_sub_; 
    
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    rclcpp::TimerBase::SharedPtr timer_;
};

#endif // PI_AMR_CONTROLLER_HPP_