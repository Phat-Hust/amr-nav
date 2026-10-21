#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <diagnostic_msgs/msg/key_value.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.hpp>
#include <tf2_ros/transform_listener.hpp>

#include "scan_safety/rectangle.hpp"

namespace scan_safety
{
using Status = diagnostic_msgs::msg::DiagnosticStatus;
using SteadyClock = std::chrono::steady_clock;

class ScanSafetyNode : public rclcpp::Node
{
public:
  explicit ScanSafetyNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("scan_safety", options),
    warning_zone_(declare_parameter<std::vector<double>>(
        "warning_zone.points", {-0.6, -0.45, 0.8, -0.45, 0.8, 0.45, -0.6, 0.45})),
    error_zone_(declare_parameter<std::vector<double>>(
        "error_zone.points", {-0.3, -0.25, 0.4, -0.25, 0.4, 0.25, -0.3, 0.25}))
  {
    scan_topic_ = declare_parameter<std::string>("scan_topic", "/scan");
    footprint_frame_ = declare_parameter<std::string>("base_footprint_frame", "base_footprint");
    base_frame_ = declare_parameter<std::string>("base_link_frame", "base_link");
    const auto diagnostics_topic = declare_parameter<std::string>(
      "diagnostics_topic", "/scan_safety/diagnostics");
    scan_timeout_ = declare_parameter<double>("scan_timeout", 0.5);
    transform_timeout_ = declare_parameter<double>("transform_timeout", 0.05);
    min_points_ = declare_parameter<int>("min_points", 1);

    if (scan_topic_.empty() || diagnostics_topic.empty() || footprint_frame_.empty() ||
      base_frame_.empty() || footprint_frame_.front() == '/' || base_frame_.front() == '/')
    {
      throw std::invalid_argument("Topics and frames must be nonempty; TF frames must not start with '/'");
    }
    if (!std::isfinite(scan_timeout_) || scan_timeout_ <= 0.0 ||
      !std::isfinite(transform_timeout_) || transform_timeout_ < 0.0 || min_points_ < 1)
    {
      throw std::invalid_argument("scan_timeout > 0, transform_timeout >= 0 and min_points >= 1 required");
    }
    if (!warning_zone_.contains(error_zone_)) {
      throw std::invalid_argument("error_zone must lie completely inside warning_zone");
    }

    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(get_clock());
    tf_listener_ = std::make_unique<tf2_ros::TransformListener>(*tf_buffer_);
    diagnostics_pub_ = create_publisher<diagnostic_msgs::msg::DiagnosticArray>(
      diagnostics_topic, rclcpp::QoS(1).reliable().transient_local());
    scan_sub_ = create_subscription<sensor_msgs::msg::LaserScan>(
      scan_topic_, rclcpp::SensorDataQoS(),
      std::bind(&ScanSafetyNode::ScanCallback, this, std::placeholders::_1));
    set_status(Status::ERROR, "Waiting for a laser scan");
    // Publish a heartbeat and detect stopped publishers even when ROS time is paused.
    watchdog_ = create_wall_timer(std::chrono::milliseconds(100), [this]() {
        if (received_scan_ &&
          std::chrono::duration<double>(SteadyClock::now() - last_scan_).count() > scan_timeout_)
        {
          warning_hits_ = 0;
          error_hits_ = 0;
          set_status(Status::ERROR, "Laser scan timeout");
        }
        publish_status();
      });
  }

private:
  tf2::Transform lookup_transform(
    const std::string & target, const std::string & source, const rclcpp::Time & stamp)
  {
    if (target == source) {
      return tf2::Transform::getIdentity();
    }
    const auto transform = tf_buffer_->lookupTransform(
      target, source, stamp, rclcpp::Duration::from_seconds(transform_timeout_));
    tf2::Transform result;
    tf2::fromMsg(transform.transform, result);
    return result;
  }

  void ScanCallback(const sensor_msgs::msg::LaserScan::ConstSharedPtr scan)
  {
    received_scan_ = true;
    last_scan_ = SteadyClock::now();
    warning_hits_ = 0;
    error_hits_ = 0;

    if (scan->header.frame_id.empty() || scan->ranges.empty() ||
      !std::isfinite(scan->angle_min) || !std::isfinite(scan->angle_increment) ||
      scan->angle_increment == 0.0f || !std::isfinite(scan->range_min) ||
      !std::isfinite(scan->range_max) || scan->range_min < 0.0f ||
      scan->range_max <= scan->range_min)
    {
      set_status(Status::ERROR, "Invalid laser scan metadata or empty ranges");
      publish_status();
      return;
    }

    const rclcpp::Time stamp(scan->header.stamp, get_clock()->get_clock_type());
    if (std::abs((now() - stamp).seconds()) > scan_timeout_) {
      set_status(Status::ERROR, "Laser scan timestamp is stale or in the future");
      publish_status();
      return;
    }

    tf2::Transform footprint_from_scan;
    try {
      // Both configured robot frames participate in the transform chain.
      const auto footprint_from_base = lookup_transform(footprint_frame_, base_frame_, stamp);
      const auto base_from_scan = lookup_transform(base_frame_, scan->header.frame_id, stamp);
      footprint_from_scan = footprint_from_base * base_from_scan;
    } catch (const tf2::TransformException & exception) {
      set_status(Status::ERROR, std::string("Scan transform unavailable: ") + exception.what());
      publish_status();
      return;
    }

    std::size_t usable_beams = 0;
    for (std::size_t i = 0; i < scan->ranges.size(); ++i) {
      const double range = scan->ranges[i];
      // Positive infinity is a valid no-return beam; it cannot hit a zone.
      if (std::isinf(range) && range > 0.0) {
        ++usable_beams;
        continue;
      }
      if (!std::isfinite(range) || range <= 0.0 ||
        range < scan->range_min || range > scan->range_max)
      {
        continue;
      }
      ++usable_beams;
      const double angle = scan->angle_min + i * static_cast<double>(scan->angle_increment);
      const tf2::Vector3 point = footprint_from_scan * tf2::Vector3(
        range * std::cos(angle), range * std::sin(angle), 0.0);
      if (warning_zone_.contains(point.x(), point.y())) {
        ++warning_hits_;
      }
      if (error_zone_.contains(point.x(), point.y())) {
        ++error_hits_;
      }
    }

    if (usable_beams == 0) {
      set_status(Status::ERROR, "Laser scan contains no usable measurements");
    } else if (error_hits_ >= min_points_) {
      set_status(Status::ERROR, "Obstacle inside error rectangle");
    } else if (warning_hits_ >= min_points_) {
      set_status(Status::WARN, "Obstacle inside warning rectangle");
    } else {
      set_status(Status::OK, "Safety rectangles clear");
    }
    publish_status();
  }

  void set_status(uint8_t level, std::string message)
  {
    if (level == level_ && message == message_) {
      return;
    }
    level_ = level;
    message_ = std::move(message);
    if (level == Status::ERROR) {
      RCLCPP_ERROR(get_logger(), "%s", message_.c_str());
    } else if (level == Status::WARN) {
      RCLCPP_WARN(get_logger(), "%s", message_.c_str());
    } else {
      RCLCPP_INFO(get_logger(), "%s", message_.c_str());
    }
  }

  void publish_status()
  {
    diagnostic_msgs::msg::DiagnosticArray notification;
    notification.header.stamp = now();
    Status status;
    status.name = std::string(get_fully_qualified_name()) + "/obstacles";
    status.hardware_id = scan_topic_;
    status.level = level_;
    status.message = message_;
    const auto add_value = [&status](const std::string & key, const std::string & value) {
        diagnostic_msgs::msg::KeyValue entry;
        entry.key = key;
        entry.value = value;
        status.values.push_back(std::move(entry));
      };
    add_value("scan_topic", scan_topic_);
    add_value("base_footprint_frame", footprint_frame_);
    add_value("base_link_frame", base_frame_);
    add_value("warning_points", std::to_string(warning_hits_));
    add_value("error_points", std::to_string(error_hits_));
    notification.status.push_back(std::move(status));
    diagnostics_pub_->publish(notification);
  }

  Rectangle warning_zone_;
  Rectangle error_zone_;
  std::string scan_topic_;
  std::string footprint_frame_;
  std::string base_frame_;
  double scan_timeout_;
  double transform_timeout_;
  int min_points_;
  int warning_hits_{0};
  int error_hits_{0};
  bool received_scan_{false};
  SteadyClock::time_point last_scan_;
  uint8_t level_{Status::ERROR};
  std::string message_;
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::unique_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr diagnostics_pub_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::TimerBase::SharedPtr watchdog_;
};
}  // namespace scan_safety

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  int result = 0;
  try {
    rclcpp::spin(std::make_shared<scan_safety::ScanSafetyNode>());
  } catch (const std::exception & exception) {
    RCLCPP_FATAL(rclcpp::get_logger("scan_safety"), "%s", exception.what());
    result = 1;
  }
  rclcpp::shutdown();
  return result;
}
