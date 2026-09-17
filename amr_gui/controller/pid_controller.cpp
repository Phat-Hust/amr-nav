// controller/pid_controller.cpp
#include "pid_controller.h"

PidController::PidController(QObject *parent) : QObject(parent) {
    m_timer.start();
    connect(&RosManager::instance(), &RosManager::telemetryUpdated,
            this, &PidController::handleTelemetry, Qt::QueuedConnection);
}

void PidController::handleTelemetry(const amr_common::msg::WheelTelemetry::SharedPtr msg) {
    double time = m_timer.elapsed() / 1000.0;
    for (int i = 0; i < amr::NUM_WHEELS; ++i) {
        m_wheels[i].addPoint(time, msg->target_velocity[i], msg->current_velocity[i]);
    }
}

void PidController::updateGains(int wheelIndex, double kp, double ki, double kd) {
    if (wheelIndex < 0 || wheelIndex >= amr::NUM_WHEELS) return;

    amr_common::msg::WheelTelemetry msg;
    msg.kp[wheelIndex] = kp;
    msg.ki[wheelIndex] = ki;
    msg.kd[wheelIndex] = kd;

    RosManager::instance().publishPidGains(msg);
}
