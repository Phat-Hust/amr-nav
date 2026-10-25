// common/ros_manager.cpp
#include "ros_manager.h"
#include "defines.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QTextStream>
#include <cmath>

RosManager& RosManager::instance() {
    static RosManager s_instance;
    return s_instance;
}

RosManager::RosManager(QObject* parent) : QObject(parent) {}

void RosManager::init(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    m_node = std::make_shared<rclcpp::Node>("amr_gui_node");

    m_telemetrySub = m_node->create_subscription<amr_common::msg::WheelTelemetry>(
            amr::TOPIC_WHEEL_TELEMETRY.toStdString(), 10,
            std::bind(&RosManager::onWheelTelemetryReceived, this, std::placeholders::_1));

    m_pidPub = m_node->create_publisher<amr_common::msg::WheelTelemetry>(
        amr::TOPIC_SET_PID.toStdString(), 10);

    m_mapSub = m_node->create_subscription<nav_msgs::msg::OccupancyGrid>(
            amr::TOPIC_MAP.toStdString(), rclcpp::QoS(rclcpp::KeepLast(1)).transient_local(),
            std::bind(&RosManager::onOccupancyGridReceived, this, std::placeholders::_1));
    m_scanSub = m_node->create_subscription<sensor_msgs::msg::LaserScan>(
            amr::TOPIC_SCAN.toStdString(), rclcpp::SensorDataQoS(),
            std::bind(&RosManager::onLaserScanReceived, this, std::placeholders::_1));

    m_finishTrajClient = m_node->create_client<cartographer_ros_msgs::srv::FinishTrajectory>("/finish_trajectory");
    m_startTrajClient  = m_node->create_client<cartographer_ros_msgs::srv::StartTrajectory>("/start_trajectory");
    m_writeStateClient = m_node->create_client<cartographer_ros_msgs::srv::WriteState>("/write_state");

}

void RosManager::spinSome() {
    if (m_node) {
        rclcpp::spin_some(m_node);
    }
}

void RosManager::publishPidGains(const amr_common::msg::WheelTelemetry& msg) {
    if (m_pidPub) {
        m_pidPub->publish(msg);
    }
}


void RosManager::shutdown() {
    if (m_node) {
        m_mapSub.reset(); m_scanSub.reset();
        m_telemetrySub.reset();
        m_pidPub.reset();
        m_finishTrajClient.reset();
        m_startTrajClient.reset();
        m_writeStateClient.reset();
        m_node.reset();
    }
    rclcpp::shutdown();
}

// Getters
int RosManager::activeMode() {
    return m_activeMode;
}

// Ros subscription callbacks
void RosManager::onWheelTelemetryReceived(const amr_common::msg::WheelTelemetry::SharedPtr msg) {
    if (msg) {
        emit handleWheelTelemetry(*msg);
    }
}

void RosManager::onOccupancyGridReceived(const nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
    if (msg) {
        emit occupancyGridReceived(*msg);
    }
}

void RosManager::onLaserScanReceived(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
    if (msg) {
        emit laserScanReceived(*msg);
    }
}

// Mode switch invokable methods (mapping or pure localization)
void RosManager::startMappingMode() {
    m_pendingMode = MODE_MAPPING;
    finishCurrentTrajectory();
}

void RosManager::startLocalizationMode(const QString &pbstreamPath) {
    m_pendingMode = MODE_LOCALIZATION;
    m_pendingPbstreamPath = pbstreamPath.toStdString();
    finishCurrentTrajectory();
}

void RosManager::saveMapState(const QString &filePath) {
    if (!m_writeStateClient || !m_writeStateClient->wait_for_service(std::chrono::milliseconds(500))) {
        RCLCPP_WARN(m_node->get_logger(), "/write_state service is unavailable");
        emit operationStatus(false, "Service /write_state unavailable");
        return;
    }

    auto request = std::make_shared<cartographer_ros_msgs::srv::WriteState::Request>();
    request->filename = filePath.toStdString();
    request->include_unfinished_submaps = true;

    m_writeStateClient->async_send_request(
        request,
        std::bind(&RosManager::onWriteStateResponse, this, std::placeholders::_1)
    );
}

void RosManager::loadMap(const QString &yamlPathStr) {
    std::string yamlPath = yamlPathStr.toStdString();
    QFileInfo yamlInfo(yamlPathStr);

    if (!yamlInfo.exists()) {
        emit operationStatus(false, "YAML file does not exist: " + yamlPathStr);
        return;
    }

    // 1. Parse YAML metadata manually (or using yaml-cpp)
    QFile yamlFile(yamlPathStr);
    if (!yamlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit operationStatus(false, "Failed to open YAML file");
        return;
    }

    QString imageFilename = "";
    double resolution = 0.05;
    double originX = 0.0;
    double originY = 0.0;
    double freeThresh = 0.196;
    double occupiedThresh = 0.65;

    QTextStream in(&yamlFile);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("image:")) {
            imageFilename = line.section("image:", 1).trimmed();
        } else if (line.startsWith("resolution:")) {
            resolution = line.section("resolution:", 1).trimmed().toDouble();
        } else if (line.startsWith("origin:")) {
            // Format: origin: [-10.0, -10.0, 0.0]
            QString originStr = line.section("origin:", 1).trimmed();
            originStr.remove('[').remove(']');
            QStringList parts = originStr.split(',');
            if (parts.size() >= 2) {
                originX = parts[0].trimmed().toDouble();
                originY = parts[1].trimmed().toDouble();
            }
        }
    }
    yamlFile.close();

    if (imageFilename.isEmpty()) {
        emit operationStatus(false, "Could not find 'image' field in YAML");
        return;
    }

    // Resolve relative path to .pgm image
    QString pgmPath = yamlInfo.absoluteDir().filePath(imageFilename);
    QImage pgmImage(pgmPath);

    if (pgmImage.isNull()) {
        emit operationStatus(false, "Failed to load PGM image: " + pgmPath);
        return;
    }

    // 2. Convert QImage (.pgm) to ROS 2 nav_msgs::msg::OccupancyGrid
    nav_msgs::msg::OccupancyGrid mapMsg;
    mapMsg.header.stamp = m_node->now();
    mapMsg.header.frame_id = "map";

    mapMsg.info.resolution = resolution;
    mapMsg.info.width = pgmImage.width();
    mapMsg.info.height = pgmImage.height();
    mapMsg.info.origin.position.x = originX;
    mapMsg.info.origin.position.y = originY;
    mapMsg.info.origin.position.z = 0.0;
    mapMsg.info.origin.orientation.w = 1.0;

    int width = pgmImage.width();
    int height = pgmImage.height();
    mapMsg.data.resize(width * height);

    // ROS OccupancyGrid stores (0,0) at bottom-left; QImage stores at top-left
    for (int y = 0; y < height; ++y) {
        int rosY = height - 1 - y; // Flip Y-axis
        for (int x = 0; x < width; ++x) {
            QRgb pixel = pgmImage.pixel(x, y);
            int gray = qGray(pixel); // Grayscale value 0-255

            // Standard ROS Map Server probability mapping
            double occ = (255.0 - gray) / 255.0;
            int8_t cellValue = -1; // Unknown

            if (occ > occupiedThresh) {
                cellValue = 100; // Occupied / Wall
            } else if (occ < freeThresh) {
                cellValue = 0;   // Free Space
            } else {
                cellValue = static_cast<int8_t>(std::round((occ - freeThresh) / (occupiedThresh - freeThresh) * 100.0));
            }

            mapMsg.data[rosY * width + x] = cellValue;
        }
    }

    // 3. Emit signal directly to MapView and publish to /map topic
    emit occupancyGridReceived(mapMsg);

    if (m_mapPub) {
        m_mapPub->publish(mapMsg);
    }

    emit operationStatus(true, "PGM Map successfully loaded");
}

void RosManager::finishCurrentTrajectory() {
    if (m_activeTrajectoryId < 0) {
        if (m_pendingMode == MODE_MAPPING) {
            requestStartMappingTrajectory();
        } else if (m_pendingMode == MODE_LOCALIZATION) {
            requestStartLocalizationTrajectory();
        }
        return;
    }

    if (!m_finishTrajClient || !m_finishTrajClient->wait_for_service(std::chrono::milliseconds(500))) {
        RCLCPP_WARN(m_node->get_logger(), "/finish_trajectory service is unavailable");
        emit operationStatus(false, "Service /finish_trajectory unavailable");
        return;
    }

    auto request = std::make_shared<cartographer_ros_msgs::srv::FinishTrajectory::Request>();
    request->trajectory_id = m_activeTrajectoryId;

    m_finishTrajClient->async_send_request(
        request,
        std::bind(&RosManager::onFinishTrajectoryResponse, this, std::placeholders::_1)
    );
}

void RosManager::onFinishTrajectoryResponse(
    rclcpp::Client<cartographer_ros_msgs::srv::FinishTrajectory>::SharedFuture future)
{
    auto response = future.get();
    if (response->status.code == 0) {
        RCLCPP_INFO(m_node->get_logger(), "Finished active trajectory ID: %d", m_activeTrajectoryId);
        m_activeTrajectoryId = -1;

        if (m_pendingMode == MODE_MAPPING) {
            requestStartMappingTrajectory();
        } else if (m_pendingMode == MODE_LOCALIZATION) {
            requestStartLocalizationTrajectory();
        }
    } else {
        RCLCPP_ERROR(m_node->get_logger(), "Failed to finish trajectory: %s", response->status.message.c_str());
        emit operationStatus(false, QString::fromStdString(response->status.message));
    }
}

void RosManager::requestStartMappingTrajectory() {
    if (!m_startTrajClient || !m_startTrajClient->wait_for_service(std::chrono::milliseconds(500))) {
        emit operationStatus(false, "Service /start_trajectory unavailable");
        return;
    }

    auto request = std::make_shared<cartographer_ros_msgs::srv::StartTrajectory::Request>();
    request->configuration_directory = qEnvironmentVariable(
        "CONFIG_FOLDER", "/var/amr/config").toStdString();
    request->configuration_basename = "amr_mapping.lua";
    request->use_initial_pose = false;

    m_startTrajClient->async_send_request(
        request,
        std::bind(&RosManager::onStartMappingResponse, this, std::placeholders::_1)
    );
}

void RosManager::onStartMappingResponse(
    rclcpp::Client<cartographer_ros_msgs::srv::StartTrajectory>::SharedFuture future)
{
    auto response = future.get();
    if (response->status.code == 0) {
        m_activeTrajectoryId = response->trajectory_id;
        m_activeMode = MODE_MAPPING;
        emit activeModeChanged();
        emit operationStatus(true, "Mapping mode started successfully");
    } else {
        emit operationStatus(false, "Failed to start mapping trajectory");
    }
}

void RosManager::requestStartLocalizationTrajectory() {
    if (!m_startTrajClient || !m_startTrajClient->wait_for_service(std::chrono::milliseconds(500))) {
        emit operationStatus(false, "Service /start_trajectory unavailable");
        return;
    }

    auto request = std::make_shared<cartographer_ros_msgs::srv::StartTrajectory::Request>();
    request->configuration_directory = qEnvironmentVariable(
        "CONFIG_FOLDER", "/var/amr/config").toStdString();
    request->configuration_basename = "amr_localization.lua";
    request->use_initial_pose = false;

    m_startTrajClient->async_send_request(
        request,
        std::bind(&RosManager::onStartLocalizationResponse, this, std::placeholders::_1)
    );
}

void RosManager::onStartLocalizationResponse(
    rclcpp::Client<cartographer_ros_msgs::srv::StartTrajectory>::SharedFuture future)
{
    auto response = future.get();
    if (response->status.code == 0) {
        m_activeTrajectoryId = response->trajectory_id;
        m_activeMode = MODE_LOCALIZATION;
        emit activeModeChanged();
        emit operationStatus(true, "Pure localization mode started successfully");
    } else {
        emit operationStatus(false, "Failed to start localization trajectory");
    }
}

void RosManager::onWriteStateResponse(
    rclcpp::Client<cartographer_ros_msgs::srv::WriteState>::SharedFuture future)
{
    auto response = future.get();
    if (response->status.code == 0) {
        RCLCPP_INFO(m_node->get_logger(), "Map state successfully saved to .pbstream");
        emit operationStatus(true, "Map saved successfully");
    } else {
        RCLCPP_ERROR(m_node->get_logger(), "Failed to save map state: %s", response->status.message.c_str());
        emit operationStatus(false, QString::fromStdString(response->status.message));
    }
}


