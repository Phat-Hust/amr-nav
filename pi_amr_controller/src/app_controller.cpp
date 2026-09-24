#include "app_controller.hpp"

PiAmrController::PiAmrController(const rclcpp::NodeOptions & options)
: Node("pi_amr_controller", options)
{
    // 1. Parameters
    this->declare_parameter<std::string>("port_name", "/dev/ttyUSB1");
    this->declare_parameter<double>("wheel_radius", 0.05);
    this->declare_parameter<double>("base_x", 0.2);
    this->declare_parameter<double>("base_y", 0.2);

    this->get_parameter("port_name", port_name_);
    this->get_parameter("wheel_radius", radius_);
    this->get_parameter("base_x", base_x_);
    this->get_parameter("base_y", base_y_);

    // 2. Init Hardware
    initSerial();

    // 3. Init Publishers & Subscribers
    odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 10);
    
    // Topic phản hồi cho GUI
    wheel_telemetry_pub_ = this->create_publisher<amr_common::msg::WheelTelemetry>("/wheel_telemetry", 10);
    cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 10,
        std::bind(&PiAmrController::cmdVelCallback, this, std::placeholders::_1));

    pid_sub_ = this->create_subscription<geometry_msgs::msg::Vector3>(
        "/gui/set_pid", 10,
        std::bind(&PiAmrController::pidCallback, this, std::placeholders::_1));

    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // 4. Timer Update (50Hz)
    last_time_ = this->now();
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(20),
        std::bind(&PiAmrController::updateLoop, this));

    RCLCPP_INFO(this->get_logger(), "PiAmrController Node started with unified wheel telemetry!");
}

PiAmrController::~PiAmrController()
{
    if (serial_fd_ != -1) {
        close(serial_fd_);
        RCLCPP_INFO(this->get_logger(), "Serial port closed.");
    }
}

void PiAmrController::initSerial()
{
    serial_fd_ = open(port_name_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd_ == -1) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open serial port: %s", port_name_.c_str());
        return;
    }

    struct termios options;
    tcgetattr(serial_fd_, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcflush(serial_fd_, TCIFLUSH);
    tcsetattr(serial_fd_, TCSANOW, &options);

    RCLCPP_INFO(this->get_logger(), "Opened and configured Serial: %s (115200)", port_name_.c_str());
}

void PiAmrController::cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    target_vx_ = msg->linear.x;
    target_vy_ = msg->linear.y;
    target_wz_ = msg->angular.z;

    // Mecanum inverse kinematics (m/s per wheel):
    // v1: Front Left, v2: Front Right, v3: Rear Left, v4: Rear Right
    double k = (base_x_ + base_y_);
    target_v_[0] = target_vx_ - target_vy_ - k * target_wz_;
    target_v_[1] = target_vx_ + target_vy_ + k * target_wz_;
    target_v_[2] = target_vx_ + target_vy_ - k * target_wz_;
    target_v_[3] = target_vx_ - target_vy_ + k * target_wz_;

    sendTargetVelocity(target_vx_, target_vy_, target_wz_);
}

void PiAmrController::pidCallback(const geometry_msgs::msg::Vector3::SharedPtr msg)
{
    sendPidParameters(msg->x, msg->y, msg->z);
    RCLCPP_INFO(this->get_logger(), "Sent PID to STM32: Kp=%.3f, Ki=%.3f, Kd=%.3f", msg->x, msg->y, msg->z);
}

void PiAmrController::sendTargetVelocity(const double &vx, const double &vy, const double &wz)
{
    if (serial_fd_ == -1) return;

    int16_t vx_int = velocityToInt16(vx);
    int16_t vy_int = velocityToInt16(vy);
    int16_t wz_int = velocityToInt16(wz);

    uint8_t frame[10];
    frame[0] = 0xAA;
    frame[1] = 0x55;
    frame[2] = 0x01;
    frame[3] = (vx_int >> 8) & 0xFF;
    frame[4] = vx_int & 0xFF;
    frame[5] = (vy_int >> 8) & 0xFF;
    frame[6] = vy_int & 0xFF;
    frame[7] = (wz_int >> 8) & 0xFF;
    frame[8] = wz_int & 0xFF;
    frame[9] = calculateCRC(frame, 9);

    write(serial_fd_, frame, sizeof(frame));
}

void PiAmrController::sendPidParameters(double kp, double ki, double kd)
{
    if (serial_fd_ == -1) return;

    int16_t kp_int = static_cast<int16_t>(kp * 1000.0);
    int16_t ki_int = static_cast<int16_t>(ki * 1000.0);
    int16_t kd_int = static_cast<int16_t>(kd * 1000.0);

    uint8_t frame[10];
    frame[0] = 0xAA;
    frame[1] = 0x55;
    frame[2] = 0x02;
    frame[3] = (kp_int >> 8) & 0xFF;
    frame[4] = kp_int & 0xFF;
    frame[5] = (ki_int >> 8) & 0xFF;
    frame[6] = ki_int & 0xFF;
    frame[7] = (kd_int >> 8) & 0xFF;
    frame[8] = kd_int & 0xFF;
    frame[9] = calculateCRC(frame, 9);

    write(serial_fd_, frame, sizeof(frame));
}

void PiAmrController::processSerialData()
{
    if (serial_fd_ == -1) return;

    uint8_t buffer[128];
    int bytes_read = read(serial_fd_, buffer, sizeof(buffer));

    if (bytes_read > 0) {
        serial_rx_buffer_.insert(serial_rx_buffer_.end(), buffer, buffer + bytes_read);
    }

    while (serial_rx_buffer_.size() >= 10) {
        if (serial_rx_buffer_[0] == 0xAA && serial_rx_buffer_[1] == 0x55) {
            uint8_t cmd = serial_rx_buffer_[2];

            // 1. Wheel velocity feedback (12 bytes)
            if (cmd == 0x01) {
                if (serial_rx_buffer_.size() < 12) break;

                uint8_t crc = calculateCRC(&serial_rx_buffer_[0], 11);
                if (crc == serial_rx_buffer_[11]) {
                    int16_t rpm_v1 = (serial_rx_buffer_[3] << 8) | serial_rx_buffer_[4];
                    int16_t rpm_v2 = (serial_rx_buffer_[5] << 8) | serial_rx_buffer_[6];
                    int16_t rpm_v3 = (serial_rx_buffer_[7] << 8) | serial_rx_buffer_[8];
                    int16_t rpm_v4 = (serial_rx_buffer_[9] << 8) | serial_rx_buffer_[10];

                    double rpm_to_m_s = (M_PI * radius_) / 30.0;
                    current_v1_ = static_cast<double>(rpm_v1) * rpm_to_m_s;
                    current_v2_ = static_cast<double>(rpm_v2) * rpm_to_m_s;
                    current_v3_ = static_cast<double>(rpm_v3) * rpm_to_m_s;
                    current_v4_ = static_cast<double>(rpm_v4) * rpm_to_m_s;

                    serial_rx_buffer_.erase(serial_rx_buffer_.begin(), serial_rx_buffer_.begin() + 12);
                    continue;
                }
            } 
            // 2. PID feedback from STM32 (10 bytes)
            else if (cmd == 0x03) {
                if (serial_rx_buffer_.size() < 10) break;

                uint8_t crc = calculateCRC(&serial_rx_buffer_[0], 9);
                if (crc == serial_rx_buffer_[9]) {
                    int16_t kp_int = (serial_rx_buffer_[3] << 8) | serial_rx_buffer_[4];
                    int16_t ki_int = (serial_rx_buffer_[5] << 8) | serial_rx_buffer_[6];
                    int16_t kd_int = (serial_rx_buffer_[7] << 8) | serial_rx_buffer_[8];

                    current_kp_ = static_cast<double>(kp_int) / 1000.0;
                    current_ki_ = static_cast<double>(ki_int) / 1000.0;
                    current_kd_ = static_cast<double>(kd_int) / 1000.0;

                    serial_rx_buffer_.erase(serial_rx_buffer_.begin(), serial_rx_buffer_.begin() + 10);
                    continue;
                }
            }
        }
        serial_rx_buffer_.erase(serial_rx_buffer_.begin());
    }
}

void PiAmrController::updateLoop()
{
    // 1. Process feedback bytes from STM32
    processSerialData();

    // 2. Mecanum forward kinematics
    double current_vx = (current_v1_ + current_v2_ + current_v3_ + current_v4_) / 4.0;
    double current_vy = (-current_v1_ + current_v2_ + current_v3_ - current_v4_) / 4.0;
    double current_wz = (-current_v1_ + current_v2_ - current_v3_ + current_v4_) / (4.0 * (base_x_ + base_y_));

    // 3. Publish unified WheelTelemetry to /wheel_telemetry
    amr_common::msg::WheelTelemetry telem_msg;
    for (int i = 0; i < 4; ++i) {
        telem_msg.target_velocity[i] = target_v_[i];
        telem_msg.kp[i] = current_kp_;
        telem_msg.ki[i] = current_ki_;
        telem_msg.kd[i] = current_kd_;
    }
    telem_msg.current_velocity[0] = current_v1_;
    telem_msg.current_velocity[1] = current_v2_;
    telem_msg.current_velocity[2] = current_v3_;
    telem_msg.current_velocity[3] = current_v4_;

    wheel_telemetry_pub_->publish(telem_msg);

    // 4. Odometry & TF Integration
    rclcpp::Time current_time = this->now();
    double dt = (current_time - last_time_).seconds();
    last_time_ = current_time;

    double delta_x = (current_vx * cos(yaw_) - current_vy * sin(yaw_)) * dt;
    double delta_y = (current_vx * sin(yaw_) + current_vy * cos(yaw_)) * dt;
    double delta_yaw = current_wz * dt;

    x_ += delta_x;
    y_ += delta_y;
    yaw_ += delta_yaw;

    tf2::Quaternion q;
    q.setRPY(0, 0, yaw_);

    geometry_msgs::msg::TransformStamped t;
    t.header.stamp = current_time;
    t.header.frame_id = "odom";
    t.child_frame_id = "base_link";
    t.transform.translation.x = x_;
    t.transform.translation.y = y_;
    t.transform.translation.z = 0.0;
    t.transform.rotation.x = q.x();
    t.transform.rotation.y = q.y();
    t.transform.rotation.z = q.z();
    t.transform.rotation.w = q.w();
    tf_broadcaster_->sendTransform(t);

    nav_msgs::msg::Odometry odom;
    odom.header.stamp = current_time;
    odom.header.frame_id = "odom";
    odom.child_frame_id = "base_link";
    odom.pose.pose.position.x = x_;
    odom.pose.pose.position.y = y_;
    odom.pose.pose.position.z = 0.0;
    odom.pose.pose.orientation.x = q.x();
    odom.pose.pose.orientation.y = q.y();
    odom.pose.pose.orientation.z = q.z();
    odom.pose.pose.orientation.w = q.w();
    odom.twist.twist.linear.x = current_vx;
    odom.twist.twist.linear.y = current_vy;
    odom.twist.twist.angular.z = current_wz;

    odom_pub_->publish(odom);
}

uint8_t PiAmrController::calculateCRC(const uint8_t* data, std::size_t length)
{
    uint8_t crc = 0x00;
    for (std::size_t i = 0; i < length; ++i) {
        crc ^= data[i];
    }
    return crc;
}

int16_t PiAmrController::velocityToInt16(double velocity_m_s)
{
    return static_cast<int16_t>(velocity_m_s * 1000.0);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<PiAmrController>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}