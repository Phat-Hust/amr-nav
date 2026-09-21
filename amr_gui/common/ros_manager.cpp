// common/ros_manager.cpp
#include "ros_manager.h"
#include "defines.h"

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
        [this](const amr_common::msg::WheelTelemetry::SharedPtr msg) {
            emit telemetryUpdated(msg);
        });

    m_pidPub = m_node->create_publisher<amr_common::msg::WheelTelemetry>(
        amr::TOPIC_SET_PID.toStdString(), 10);

//    m_emerBtn = m_node->create_subscription<std_msgs::msg::Int16>(
//        amr::TOPIC_EMERGENCY_BUTTON.toStdString(), 10,
//                [this](const std_msgs::msg::Int16::SharedPtr msg) {
//            emit EmegencyUpdated(msg);
//    });
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
    rclcpp::shutdown();
}
