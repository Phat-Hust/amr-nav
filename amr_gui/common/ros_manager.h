// common/ros_manager.h
#pragma once
#include <QObject>
#include <QString>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include "amr_common/msg/wheel_telemetry.hpp"
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <cartographer_ros_msgs/srv/finish_trajectory.hpp>
#include <cartographer_ros_msgs/srv/start_trajectory.hpp>
#include <cartographer_ros_msgs/srv/write_state.hpp>

class RosManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int activeMode READ activeMode NOTIFY activeModeChanged)

public:
    enum RobotMode {
        MODE_IDLE = 0,
        MODE_MAPPING = 1,
        MODE_LOCALIZATION = 2
    };
    Q_ENUM(RobotMode)

    static RosManager& instance();

    // Getters for Q_PROPERTY
    int activeMode();

    void init(int argc, char *argv[]);
    void spinSome();
    void shutdown();

    std::shared_ptr<rclcpp::Node> getNode() const { return m_node; }
    void publishPidGains(const amr_common::msg::WheelTelemetry& msg);

    // Q_INVOKALE methods
    Q_INVOKABLE void startMappingMode();
    Q_INVOKABLE void startLocalizationMode(const QString &pbstreamPath);
    Q_INVOKABLE void saveMapState(const QString &filePath);
    Q_INVOKABLE void loadMap(const QString &yamlPath);

signals:
    void handleRobotPose(double x, double y, double yaw);
    void handleWheelTelemetry(const amr_common::msg::WheelTelemetry msg);
    void occupancyGridReceived(const nav_msgs::msg::OccupancyGrid &mapMsg);
    void laserScanReceived(const sensor_msgs::msg::LaserScan &scanMsg);
    // Notify
    void activeModeChanged();
    void operationStatus(bool success, const QString& message);

private:
    explicit RosManager(QObject* parent = nullptr);
    ~RosManager() override = default;

    // Callback methods
    // Callback Methods
    void onWheelTelemetryReceived(const amr_common::msg::WheelTelemetry::SharedPtr msg);
    void onOccupancyGridReceived(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);
    void onLaserScanReceived(const sensor_msgs::msg::LaserScan::SharedPtr msg);

    // Response Callbacks
    void onFinishTrajectoryResponse(
            rclcpp::Client<cartographer_ros_msgs::srv::FinishTrajectory>::SharedFuture future);
    void onStartMappingResponse(
            rclcpp::Client<cartographer_ros_msgs::srv::StartTrajectory>::SharedFuture future);
    void onStartLocalizationResponse(
            rclcpp::Client<cartographer_ros_msgs::srv::StartTrajectory>::SharedFuture future);
    void onWriteStateResponse(
            rclcpp::Client<cartographer_ros_msgs::srv::WriteState>::SharedFuture future);
    void finishCurrentTrajectory();
    void requestStartMappingTrajectory();
    void requestStartLocalizationTrajectory();

    std::shared_ptr<rclcpp::Node> m_node;
    rclcpp::Subscription<amr_common::msg::WheelTelemetry>::SharedPtr m_telemetrySub;
    rclcpp::Publisher<amr_common::msg::WheelTelemetry>::SharedPtr m_pidPub;
    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr m_mapSub;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr m_scanSub;
    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr m_mapPub;

    rclcpp::Client<cartographer_ros_msgs::srv::FinishTrajectory>::SharedPtr m_finishTrajClient;
    rclcpp::Client<cartographer_ros_msgs::srv::StartTrajectory>::SharedPtr m_startTrajClient;
    rclcpp::Client<cartographer_ros_msgs::srv::WriteState>::SharedPtr m_writeStateClient;

    int m_activeTrajectoryId{-1};
    int m_activeMode{MODE_IDLE};

    RobotMode m_pendingMode{MODE_IDLE};
    std::string m_pendingPbstreamPath;
//    rclcpp::Subscription<std_msgs::msg::Int16>::SharedPtr m_emerBtn;
};
