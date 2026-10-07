// common/ros_manager.h
#pragma once
#include <QObject>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include "amr_common/msg/wheel_telemetry.hpp"

class RosManager : public QObject {
    Q_OBJECT

public:
    static RosManager& instance();

    void init(int argc, char *argv[]);
    void spinSome();
    void shutdown();

    std::shared_ptr<rclcpp::Node> getNode() const { return m_node; }
    void publishPidGains(const amr_common::msg::WheelTelemetry& msg);

signals:
    void handleRobotPose(double x, double y, double yaw);
    void handleWheelTelemetry(const amr_common::msg::WheelTelemetry msg);
    void handleOccupancyGridMap(const nav_msgs::msg::OccupancyGrid &mapMsg);
    void handleLaserScan(const sensor_msgs::msg::LaserScan &scanMsg);

private:
    explicit RosManager(QObject* parent = nullptr);
    ~RosManager() override = default;

    std::shared_ptr<rclcpp::Node> m_node;
    rclcpp::Subscription<amr_common::msg::WheelTelemetry>::SharedPtr m_telemetrySub;
    rclcpp::Publisher<amr_common::msg::WheelTelemetry>::SharedPtr m_pidPub;
//    rclcpp::Subscription<std_msgs::msg::Int16>::SharedPtr m_emerBtn;
};
