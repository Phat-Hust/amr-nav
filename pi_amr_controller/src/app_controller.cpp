#include "pi_amr_controller/app_controller.hpp"   // vif header hiện tại nằm tại include/pi_amr_controller/app_controller.hpp

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>

// Linux serial
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// ROS2 TF
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/LinearMath/Quaternion.h"

using namespace std::chrono_literals;


/* =========================================================
 * Constructor
 * ========================================================= */
AppController::AppController(
  const rclcpp::NodeOptions & options
)
: Node("app_controller", options)
{
  /* ---------------- Parameters ---------------- */

  this->declare_parameter<std::string>(
    "serial_port",
    "/dev/ttyACM2"
  );

  this->declare_parameter<double>(
    "wheel_radius",
    0.0485
  );

  this->declare_parameter<double>(
    "wheel_base_x",
    0.25
  );

  this->declare_parameter<double>(
    "wheel_base_y",
    0.25
  );

  this->declare_parameter<double>(
    "max_wheel_speed_mps",
    2.0
  );

  this->declare_parameter<double>(
    "cmd_vel_timeout",
    0.5
  );

  this->declare_parameter<double>(
    "longitudinal_gain",
    -1.0
  );

  this->declare_parameter<double>(
    "lateral_gain",
    1.0
  );

  this->declare_parameter<double>(
    "angular_gain",
    -1.0
  );

  /* ---------------- Read parameters ---------------- */

  port_name_ =
    this->get_parameter("serial_port").as_string();

  r_ =
    this->get_parameter("wheel_radius").as_double();

  lx_ =
    this->get_parameter("wheel_base_x").as_double();

  ly_ =
    this->get_parameter("wheel_base_y").as_double();

  max_wheel_speed_mps_ =
    this->get_parameter(
      "max_wheel_speed_mps"
    ).as_double();

  cmd_vel_timeout_s_ =
    this->get_parameter(
      "cmd_vel_timeout"
    ).as_double();

  longitudinal_gain_ =
    this->get_parameter(
      "longitudinal_gain"
    ).as_double();

  lateral_gain_ =
    this->get_parameter(
      "lateral_gain"
    ).as_double();

  angular_gain_ =
    this->get_parameter(
      "angular_gain"
    ).as_double();

  /* ---------------- Validate parameters ---------------- */

  if (!std::isfinite(r_) || r_ <= 0.0)
  {
    RCLCPP_WARN(
      this->get_logger(),
      "Invalid wheel_radius. Using 0.0485 m."
    );

    r_ = 0.0485;
  }

  if (!std::isfinite(lx_) || lx_ < 0.0)
  {
    lx_ = 0.25;
  }

  if (!std::isfinite(ly_) || ly_ < 0.0)
  {
    ly_ = 0.25;
  }

  if ((lx_ + ly_) <= 0.0)
  {
    RCLCPP_WARN(
      this->get_logger(),
      "Invalid wheel base. Using 0.25 m, 0.25 m."
    );

    lx_ = 0.25;
    ly_ = 0.25;
  }

  if (!std::isfinite(max_wheel_speed_mps_) ||
      max_wheel_speed_mps_ <= 0.0)
  {
    RCLCPP_WARN(
      this->get_logger(),
      "Invalid max wheel speed. Using 2.0 m/s."
    );

    max_wheel_speed_mps_ = 2.0;
  }

  /*
   * int16_t mm/s chỉ biểu diễn tối đa
   * khoảng 32.767 m/s.
   */
  if (max_wheel_speed_mps_ > 32.767)
  {
    RCLCPP_WARN(
      this->get_logger(),
      "max_wheel_speed_mps is too large. "
      "Limited to 32.767 m/s."
    );

    max_wheel_speed_mps_ = 32.767;
  }

  if (!std::isfinite(cmd_vel_timeout_s_) ||
      cmd_vel_timeout_s_ <= 0.0)
  {
    RCLCPP_WARN(
      this->get_logger(),
      "Invalid cmd_vel timeout. Using 0.5 s."
    );

    cmd_vel_timeout_s_ = 0.5;
  }

  /* ---------------- ROS2 objects ---------------- */

  odom_pub_ =
    this->create_publisher<nav_msgs::msg::Odometry>(
      "/odom",
      10
    );

  tf_broadcaster_ =
    std::make_unique<tf2_ros::TransformBroadcaster>(
      *this
    );

  cmd_vel_sub_ =
    this->create_subscription<geometry_msgs::msg::Twist>(
      "/cmd_vel",
      10,
      std::bind(
        &AppController::cmd_vel_callback,
        this,
        std::placeholders::_1
      )
    );

  /* ---------------- Initial state ---------------- */

  target_vx_ = 0.0;
  target_vy_ = 0.0;
  target_wz_ = 0.0;

  x_ = 0.0;
  y_ = 0.0;
  theta_ = 0.0;

  const rclcpp::Time now =
    this->get_clock()->now();

  last_time_ = now;
  last_cmd_vel_time_ = now;

  /* ---------------- Serial ---------------- */

  init_serial();

  /* ---------------- Main timer: 50 Hz ---------------- */

  timer_ = this->create_wall_timer(
    20ms,
    std::bind(
      &AppController::update_loop,
      this
    )
  );

  RCLCPP_INFO(
    this->get_logger(),
    "AppController initialized | "
    "port=%s radius=%.4f m "
    "max_wheel_speed=%.3f m/s "
    "cmd_timeout=%.3f s",
    port_name_.c_str(),
    r_,
    max_wheel_speed_mps_,
    cmd_vel_timeout_s_
  );
}


/* =========================================================
 * Destructor
 * ========================================================= */
AppController::~AppController()
{
  if (serial_fd_ >= 0)
  {
    /*
     * Gửi một frame dừng trước khi đóng serial.
     */
    send_hardware_velocities(
      0.0,
      0.0,
      0.0,
      0.0
    );

    tcdrain(serial_fd_);

    ::close(serial_fd_);
    serial_fd_ = -1;

    RCLCPP_INFO(
      this->get_logger(),
      "Serial port closed"
    );
  }
}


/* =========================================================
 * Initialize serial port
 * ========================================================= */
void AppController::init_serial()
{
  serial_fd_ = ::open(
    port_name_.c_str(),
    O_RDWR | O_NOCTTY | O_NDELAY
  );

  if (serial_fd_ < 0)
  {
    RCLCPP_ERROR(
      this->get_logger(),
      "Failed to open serial port %s: %s",
      port_name_.c_str(),
      std::strerror(errno)
    );

    return;
  }

  /*
   * Chuyển file descriptor sang blocking mode.
   * VMIN và VTIME vẫn được cấu hình cho read không chờ.
   */
  if (fcntl(serial_fd_, F_SETFL, 0) < 0)
  {
    RCLCPP_WARN(
      this->get_logger(),
      "Failed to set serial blocking mode: %s",
      std::strerror(errno)
    );
  }

  struct termios tty;
  std::memset(&tty, 0, sizeof(tty));

  if (tcgetattr(serial_fd_, &tty) != 0)
  {
    RCLCPP_ERROR(
      this->get_logger(),
      "Failed to get serial attributes: %s",
      std::strerror(errno)
    );

    ::close(serial_fd_);
    serial_fd_ = -1;

    return;
  }

  /* ---------------- Baud rate ---------------- */

  if (cfsetispeed(&tty, B115200) != 0 ||
      cfsetospeed(&tty, B115200) != 0)
  {
    RCLCPP_ERROR(
      this->get_logger(),
      "Failed to set serial baud rate"
    );

    ::close(serial_fd_);
    serial_fd_ = -1;

    return;
  }

  /* ---------------- 8N1 ---------------- */

  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;

#ifdef CRTSCTS
  tty.c_cflag &= ~CRTSCTS;
#endif

  tty.c_cflag |= CLOCAL;
  tty.c_cflag |= CREAD;

  /* ---------------- Raw input ---------------- */

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ECHOE;
  tty.c_lflag &= ~ISIG;

  tty.c_iflag &= ~IXON;
  tty.c_iflag &= ~IXOFF;
  tty.c_iflag &= ~IXANY;

  tty.c_iflag &= ~IGNBRK;
  tty.c_iflag &= ~BRKINT;
  tty.c_iflag &= ~PARMRK;
  tty.c_iflag &= ~ISTRIP;
  tty.c_iflag &= ~INLCR;
  tty.c_iflag &= ~IGNCR;
  tty.c_iflag &= ~ICRNL;

  /* ---------------- Raw output ---------------- */

  tty.c_oflag &= ~OPOST;

#ifdef ONLCR
  tty.c_oflag &= ~ONLCR;
#endif

  /*
   * Read không chặn:
   * nếu không có dữ liệu thì read() trả về ngay.
   */
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;

  if (tcsetattr(
        serial_fd_,
        TCSANOW,
        &tty
      ) != 0)
  {
    RCLCPP_ERROR(
      this->get_logger(),
      "Failed to set serial attributes: %s",
      std::strerror(errno)
    );

    ::close(serial_fd_);
    serial_fd_ = -1;

    return;
  }

  tcflush(serial_fd_, TCIOFLUSH);

  RCLCPP_INFO(
    this->get_logger(),
    "Serial port opened: %s at 115200 baud",
    port_name_.c_str()
  );
}


/* =========================================================
 * CRC XOR
 *
 * CRC = XOR của tất cả byte trước CRC.
 * ========================================================= */
uint8_t AppController::calculate_crc(
  const uint8_t *data,
  std::size_t length
)
{
  uint8_t crc = 0;

  for (std::size_t i = 0; i < length; ++i)
  {
    crc ^= data[i];
  }

  return crc;
}


/* =========================================================
 * Convert velocity
 *
 * m/s -> mm/s -> int16_t
 *
 * Giới hạn cuối cùng:
 * max_wheel_speed_mps_
 * ========================================================= */
int16_t AppController::velocity_to_int16(
  double velocity_m_s
)
{
  if (!std::isfinite(velocity_m_s))
  {
    return 0;
  }

  long velocity_mm_s =
    std::lround(
      velocity_m_s * 1000.0
    );

  long max_velocity_mm_s =
    std::lround(
      std::fabs(max_wheel_speed_mps_) * 1000.0
    );

  const long int16_max =
    static_cast<long>(
      std::numeric_limits<int16_t>::max()
    );

  if (max_velocity_mm_s > int16_max)
  {
    max_velocity_mm_s = int16_max;
  }

  if (max_velocity_mm_s < 1)
  {
    max_velocity_mm_s = 1;
  }

  if (velocity_mm_s > max_velocity_mm_s)
  {
    velocity_mm_s = max_velocity_mm_s;
  }
  else if (velocity_mm_s < -max_velocity_mm_s)
  {
    velocity_mm_s = -max_velocity_mm_s;
  }

  return static_cast<int16_t>(
    velocity_mm_s
  );
}


/* =========================================================
 * Send binary frame
 *
 * Frame 11 bytes:
 *
 * byte 0  : 0xAA
 * byte 1  : 0x55
 *
 * byte 2  : v1 low
 * byte 3  : v1 high
 *
 * byte 4  : v2 low
 * byte 5  : v2 high
 *
 * byte 6  : v3 low
 * byte 7  : v3 high
 *
 * byte 8  : v4 low
 * byte 9  : v4 high
 *
 * byte 10 : CRC XOR byte 0 -> byte 9
 *
 * v1...v4:
 * int16_t, đơn vị mm/s, little-endian
 * ========================================================= */
bool AppController::send_hardware_velocities(
  double v1,
  double v2,
  double v3,
  double v4
)
{
  if (serial_fd_ < 0)
  {
    RCLCPP_WARN_THROTTLE(
      this->get_logger(),
      *this->get_clock(),
      1000,
      "Cannot send frame: serial port is closed"
    );

    return false;
  }

  constexpr uint8_t HEADER_1 = 0xAA;
  constexpr uint8_t HEADER_2 = 0x55;

  const int16_t speed1 =
    velocity_to_int16(v1);

  const int16_t speed2 =
    velocity_to_int16(v2);

  const int16_t speed3 =
    velocity_to_int16(v3);

  const int16_t speed4 =
    velocity_to_int16(v4);

  std::array<uint8_t, 11> frame{};

  frame[0] = HEADER_1;
  frame[1] = HEADER_2;

  const auto pack_int16 =
    [&frame](
      std::size_t index,
      int16_t value
    )
    {
      const uint16_t raw =
        static_cast<uint16_t>(value);

      frame[index] =
        static_cast<uint8_t>(
          raw & 0x00FFU
        );

      frame[index + 1] =
        static_cast<uint8_t>(
          (raw >> 8U) & 0x00FFU
        );
    };

  pack_int16(2, speed1);
  pack_int16(4, speed2);
  pack_int16(6, speed3);
  pack_int16(8, speed4);

  /*
   * CRC của 10 byte đầu.
   */
  frame[10] = calculate_crc(
    frame.data(),
    10
  );

  std::size_t total_written = 0;

  while (total_written < frame.size())
  {
    const ssize_t bytes_written =
      ::write(
        serial_fd_,
        frame.data() + total_written,
        frame.size() - total_written
      );

    if (bytes_written < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }

      RCLCPP_ERROR(
        this->get_logger(),
        "Failed to send binary frame: %s",
        std::strerror(errno)
      );

      return false;
    }

    if (bytes_written == 0)
    {
      RCLCPP_ERROR(
        this->get_logger(),
        "Serial write returned zero bytes"
      );

      return false;
    }

    total_written +=
      static_cast<std::size_t>(
        bytes_written
      );
  }

  // RCLCPP_INFO_THROTTLE(
  //   this->get_logger(),
  //   *this->get_clock(),
  //   1000,
  //   "Binary TX: "
  //   "%02X %02X "
  //   "%02X %02X "
  //   "%02X %02X "
  //   "%02X %02X "
  //   "%02X %02X "
  //   "%02X | "
  //   "mm/s: %d %d %d %d",
  //   static_cast<unsigned int>(frame[0]),
  //   static_cast<unsigned int>(frame[1]),
  //   static_cast<unsigned int>(frame[2]),
  //   static_cast<unsigned int>(frame[3]),
  //   static_cast<unsigned int>(frame[4]),
  //   static_cast<unsigned int>(frame[5]),
  //   static_cast<unsigned int>(frame[6]),
  //   static_cast<unsigned int>(frame[7]),
  //   static_cast<unsigned int>(frame[8]),
  //   static_cast<unsigned int>(frame[9]),
  //   static_cast<unsigned int>(frame[10]),
  //   static_cast<int>(speed1),
  //   static_cast<int>(speed2),
  //   static_cast<int>(speed3),
  //   static_cast<int>(speed4)
  // );

  return true;
}


/* =========================================================
 * Receive /cmd_vel
 * ========================================================= */
void AppController::cmd_vel_callback(
  const geometry_msgs::msg::Twist::SharedPtr msg
)
{
  target_vx_ = msg->linear.x;
  target_vy_ = msg->linear.y;
  target_wz_ = msg->angular.z;

  last_cmd_vel_time_ =
    this->get_clock()->now();

  // RCLCPP_INFO_THROTTLE(
  //   this->get_logger(),
  //   *this->get_clock(),
  //   1000,
  //   "Received cmd_vel: "
  //   "vx=%.3f vy=%.3f wz=%.3f",
  //   target_vx_,
  //   target_vy_,
  //   target_wz_
  // );
}


/* =========================================================
 * Mecanum inverse kinematics
 *
 * Input:
 * vx, vy : m/s
 * wz     : rad/s
 *
 * Output:
 * w1...w4: rad/s
 * ========================================================= */
void AppController::compute_wheel_targets(
  double vx,
  double vy,
  double wz,
  double &w1,
  double &w2,
  double &w3,
  double &w4
)
{
  if (r_ <= 0.0)
  {
    w1 = 0.0;
    w2 = 0.0;
    w3 = 0.0;
    w4 = 0.0;

    RCLCPP_ERROR_THROTTLE(
      this->get_logger(),
      *this->get_clock(),
      1000,
      "Invalid wheel radius: %.4f",
      r_
    );

    return;
  }

  const double L = lx_ + ly_;

  const double vx_comp =
    vx * longitudinal_gain_;

  const double vy_comp =
    vy * lateral_gain_;

  const double wz_comp =
    wz * angular_gain_;

  w1 =
    (vx_comp - vy_comp - L * wz_comp) / r_;

  w2 =
    (vx_comp + vy_comp + L * wz_comp) / r_;

  w3 =
    (vx_comp + vy_comp - L * wz_comp) / r_;

  w4 =
    (vx_comp - vy_comp + L * wz_comp) / r_;
}


/* =========================================================
 * Convert rad/s to m/s and send
 *
 * Nếu một bánh vượt giới hạn, tất cả bánh được scale
 * cùng tỷ lệ để giữ đúng hướng chuyển động.
 * ========================================================= */
void AppController::send_wheel_targets(
  double w1,
  double w2,
  double w3,
  double w4
)
{
  if (serial_fd_ < 0)
  {
    return;
  }

  double v1 = w1 * r_;
  double v2 = w2 * r_;
  double v3 = w3 * r_;
  double v4 = w4 * r_;

  const double largest_velocity =
    std::max(
      std::max(
        std::fabs(v1),
        std::fabs(v2)
      ),
      std::max(
        std::fabs(v3),
        std::fabs(v4)
      )
    );

  if (largest_velocity >
        max_wheel_speed_mps_ &&
      largest_velocity > 0.0)
  {
    const double scale =
      max_wheel_speed_mps_ /
      largest_velocity;

    v1 *= scale;
    v2 *= scale;
    v3 *= scale;
    v4 *= scale;

    RCLCPP_WARN_THROTTLE(
      this->get_logger(),
      *this->get_clock(),
      1000,
      "Wheel commands scaled by %.3f "
      "to respect %.3f m/s limit",
      scale,
      max_wheel_speed_mps_
    );
  }

  send_hardware_velocities(
    v1,
    v2,
    v3,
    v4
  );
}


/* =========================================================
 * Read actual wheel velocities from STM32
 *
 * Chưa triển khai feedback STM32 -> ROS2.
 *
 * Output trong tương lai phải là rad/s.
 * ========================================================= */
bool AppController::read_hardware_velocities(
  double &v1,
  double &v2,
  double &v3,
  double &v4
)
{
  if (serial_fd_ < 0)
  {
    return false;
  }

  if (r_ <= 0.0)
  {
    return false;
  }

  constexpr uint8_t FEEDBACK_HEADER_1 = 0xAB;
  constexpr uint8_t FEEDBACK_HEADER_2 = 0xCD;
  constexpr std::size_t FEEDBACK_FRAME_SIZE = 11;

  /*
    Đọc tất cả byte hiện có trong serial.
  */
  uint8_t temporary_buffer[64];

  while (true)
  {
    const ssize_t bytes_read =
      ::read(
        serial_fd_,
        temporary_buffer,
        sizeof(temporary_buffer)
      );

    if (bytes_read > 0)
    {
      serial_rx_buffer_.insert(
        serial_rx_buffer_.end(),
        temporary_buffer,
        temporary_buffer + bytes_read
      );

      /*
        Tránh buffer tăng vô hạn khi dữ liệu lỗi.
      */
      if (serial_rx_buffer_.size() > 512)
      {
        const std::size_t remove_count =
          serial_rx_buffer_.size() - 512;

        serial_rx_buffer_.erase(
          serial_rx_buffer_.begin(),
          serial_rx_buffer_.begin() +
          static_cast<std::ptrdiff_t>(remove_count)
        );
      }

      continue;
    }

    if (bytes_read == 0)
    {
      break;
    }

    if (errno == EINTR)
    {
      continue;
    }

    if (errno == EAGAIN ||
        errno == EWOULDBLOCK)
    {
      break;
    }

    RCLCPP_WARN_THROTTLE(
      this->get_logger(),
      *this->get_clock(),
      1000,
      "Serial read error: %s",
      std::strerror(errno)
    );

    return false;
  }

  bool valid_feedback_received = false;

  /*
    Giải mã tất cả frame đang có.
    Nếu có nhiều frame, giữ frame mới nhất.
  */
  while (serial_rx_buffer_.size() >= 2)
  {
    /*
      Tìm đúng header AB CD.
    */
    if (serial_rx_buffer_[0] !=
          FEEDBACK_HEADER_1 ||
        serial_rx_buffer_[1] !=
          FEEDBACK_HEADER_2)
    {
      serial_rx_buffer_.erase(
        serial_rx_buffer_.begin()
      );

      continue;
    }

    /*
      Có header nhưng chưa nhận đủ 11 byte.
    */
    if (serial_rx_buffer_.size() <
        FEEDBACK_FRAME_SIZE)
    {
      break;
    }

    const uint8_t received_crc =
      serial_rx_buffer_[10];

    const uint8_t calculated_crc =
      calculate_crc(
        serial_rx_buffer_.data(),
        10
      );

    if (received_crc != calculated_crc)
    {
      RCLCPP_WARN_THROTTLE(
        this->get_logger(),
        *this->get_clock(),
        1000,
        "Invalid feedback CRC"
      );

      /*
        Bỏ một byte và tìm lại header.
      */
      serial_rx_buffer_.erase(
        serial_rx_buffer_.begin()
      );

      continue;
    }

    const auto decode_int16_le =
      [](
        uint8_t low_byte,
        uint8_t high_byte
      ) -> int16_t
      {
        const uint16_t raw =
          static_cast<uint16_t>(low_byte) |
          (
            static_cast<uint16_t>(high_byte)
            << 8U
          );

        return static_cast<int16_t>(raw);
      };

    const int16_t speed1_mmps =
      decode_int16_le(
        serial_rx_buffer_[2],
        serial_rx_buffer_[3]
      );

    const int16_t speed2_mmps =
      decode_int16_le(
        serial_rx_buffer_[4],
        serial_rx_buffer_[5]
      );

    const int16_t speed3_mmps =
      decode_int16_le(
        serial_rx_buffer_[6],
        serial_rx_buffer_[7]
      );

    const int16_t speed4_mmps =
      decode_int16_le(
        serial_rx_buffer_[8],
        serial_rx_buffer_[9]
      );

    /*
      STM32 gửi tốc độ dài m/s.
      Odometry cần tốc độ góc rad/s.

      omega = v / r
    */
    v1 =
      (
        static_cast<double>(speed1_mmps) /
        1000.0
      ) / r_;

    v2 =
      (
        static_cast<double>(speed2_mmps) /
        1000.0
      ) / r_;

    v3 =
      (
        static_cast<double>(speed3_mmps) /
        1000.0
      ) / r_;

    v4 =
      (
        static_cast<double>(speed4_mmps) /
        1000.0
      ) / r_;

    valid_feedback_received = true;

    /*
      Xóa frame đã xử lý.
    */
    serial_rx_buffer_.erase(
      serial_rx_buffer_.begin(),
      serial_rx_buffer_.begin() +
      static_cast<std::ptrdiff_t>(
        FEEDBACK_FRAME_SIZE
      )
    );
  }

  if (valid_feedback_received)
  {
    const double wheel1_mps = v1 * r_;
    const double wheel2_mps = v2 * r_;
    const double wheel3_mps = v3 * r_;
    const double wheel4_mps = v4 * r_;

  RCLCPP_INFO_THROTTLE(
    this->get_logger(),
    *this->get_clock(),
    1000,
    "Actual wheel speed m/s: "
    "M1=%.3f M2=%.3f M3=%.3f M4=%.3f",
    wheel1_mps,
    wheel2_mps,
    wheel3_mps,
    wheel4_mps
 );
  }

  return valid_feedback_received;
}


/* =========================================================
 * Main update loop: 50 Hz
 * ========================================================= */
void AppController::update_loop()
{
  const rclcpp::Time current_time =
    this->get_clock()->now();

  /* ---------------- ROS2 command watchdog ---------------- */

  const double cmd_age =
    (current_time - last_cmd_vel_time_).seconds();

  if (cmd_age > cmd_vel_timeout_s_)
  {
    const bool robot_was_commanded =
      std::fabs(target_vx_) > 1e-6 ||
      std::fabs(target_vy_) > 1e-6 ||
      std::fabs(target_wz_) > 1e-6;

    if (robot_was_commanded)
    {
      RCLCPP_WARN(
        this->get_logger(),
        "cmd_vel timeout after %.3f s. "
        "Stopping robot.",
        cmd_age
      );
    }

    target_vx_ = 0.0;
    target_vy_ = 0.0;
    target_wz_ = 0.0;
  }

  /* ---------------- Inverse kinematics ---------------- */

  double w1_cmd = 0.0;
  double w2_cmd = 0.0;
  double w3_cmd = 0.0;
  double w4_cmd = 0.0;

  compute_wheel_targets(
    target_vx_,
    target_vy_,
    target_wz_,
    w1_cmd,
    w2_cmd,
    w3_cmd,
    w4_cmd
  );

  send_wheel_targets(
    w1_cmd,
    w2_cmd,
    w3_cmd,
    w4_cmd
  );

  RCLCPP_INFO_THROTTLE(
    this->get_logger(),
    *this->get_clock(),
    1000,
    "Target: vx=%.3f vy=%.3f wz=%.3f | "
    "wheel rad/s: %.3f %.3f %.3f %.3f",
    target_vx_,
    target_vy_,
    target_wz_,
    w1_cmd,
    w2_cmd,
    w3_cmd,
    w4_cmd
  );

  /* ---------------- Read feedback ---------------- */

  double u1 = 0.0;
  double u2 = 0.0;
  double u3 = 0.0;
  double u4 = 0.0;

  if (!read_hardware_velocities(
        u1,
        u2,
        u3,
        u4
      ))
  {
    last_time_ = current_time;
    return;
  }

  /* ---------------- Time step ---------------- */

  const double dt =
    (current_time - last_time_).seconds();

  if (!std::isfinite(dt) || dt <= 0.0)
  {
    last_time_ = current_time;
    return;
  }

  const double L = lx_ + ly_;

  if (L <= 0.0)
  {
    RCLCPP_ERROR_THROTTLE(
      this->get_logger(),
      *this->get_clock(),
      1000,
      "Invalid robot dimensions: lx + ly <= 0"
    );

    last_time_ = current_time;
    return;
  }

  /* ---------------- Forward kinematics ---------------- */

  const double vx =
    (r_ / 4.0) *
    (u1 + u2 + u3 + u4);

  const double vy =
    (r_ / 4.0) *
    (-u1 + u2 + u3 - u4);

  const double wz =
    (r_ / (4.0 * L)) *
    (-u1 + u2 - u3 + u4);

  
  RCLCPP_INFO_THROTTLE(
  this->get_logger(),
  *this->get_clock(),
  1000,
  "Actual robot velocity: "
  "vx=%.3f m/s vy=%.3f m/s wz=%.3f rad/s",
  vx,
  vy,
  wz
 );
 /* ---------------- Integrate odometry ---------------- */
  const double delta_x =
    (
      vx * std::cos(theta_) -
      vy * std::sin(theta_)
    ) * dt;

  const double delta_y =
    (
      vx * std::sin(theta_) +
      vy * std::cos(theta_)
    ) * dt;

  const double delta_theta =
    wz * dt;

  x_ += delta_x;
  y_ += delta_y;
  theta_ += delta_theta;

  theta_ =
    std::atan2(
      std::sin(theta_),
      std::cos(theta_)
    );

  /* ---------------- Quaternion ---------------- */

  tf2::Quaternion quaternion;

  quaternion.setRPY(
    0.0,
    0.0,
    theta_
  );

  /* ---------------- TF odom -> base_footprint ---------------- */

  geometry_msgs::msg::TransformStamped tf_msg;

  tf_msg.header.stamp = current_time;
  tf_msg.header.frame_id = "odom";
  tf_msg.child_frame_id = "base_footprint";

  tf_msg.transform.translation.x = x_;
  tf_msg.transform.translation.y = y_;
  tf_msg.transform.translation.z = 0.0;

  tf_msg.transform.rotation.x = quaternion.x();
  tf_msg.transform.rotation.y = quaternion.y();
  tf_msg.transform.rotation.z = quaternion.z();
  tf_msg.transform.rotation.w = quaternion.w();

  tf_broadcaster_->sendTransform(tf_msg);

  /* ---------------- Odometry message ---------------- */

  nav_msgs::msg::Odometry odom_msg;

  odom_msg.header.stamp = current_time;
  odom_msg.header.frame_id = "odom";
  odom_msg.child_frame_id = "base_footprint";

  odom_msg.pose.pose.position.x = x_;
  odom_msg.pose.pose.position.y = y_;
  odom_msg.pose.pose.position.z = 0.0;

  odom_msg.pose.pose.orientation =
    tf_msg.transform.rotation;

  odom_msg.twist.twist.linear.x = vx;
  odom_msg.twist.twist.linear.y = vy;
  odom_msg.twist.twist.angular.z = wz;

  odom_pub_->publish(odom_msg);

  last_time_ = current_time;
}

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<AppController>());
  rclcpp::shutdown();
  return 0;
}