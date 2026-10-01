// controller/pid_controller.cpp
#include "pid_controller.h"

PidController::PidController(QObject *parent) : QObject(parent) {
    for (auto &model : m_wheelModels) {
            model.setParent(this);
        }
    for (int i = 0; i < amr::NUM_WHEELS; ++i) {
        QString idx = QString::number(i);
        // Feedback initialization from ros
        m_wheelTelemetry["target_vel_" + idx]  = 0.0;
        m_wheelTelemetry["current_vel_" + idx] = 0.0;
        m_wheelTelemetry["kp_" + idx]          = 1.0;
        m_wheelTelemetry["ki_" + idx]          = 0.1;
        m_wheelTelemetry["kd_" + idx]          = 0.05;

        // UI Setpoint
        m_setWheelTelemetry["target_vel_" + idx] = 0.0;
        m_setWheelTelemetry["kp_" + idx]         = 1.0;
        m_setWheelTelemetry["ki_" + idx]         = 0.1;
        m_setWheelTelemetry["kd_" + idx]         = 0.05;
    }
}

void PidController::updateWheelTelemetry(const amr_common::msg::WheelTelemetry &msg)
{
    double time = m_timer.elapsed() / 1000.0;
    for (int i = 0; i < amr::NUM_WHEELS; ++i) {
        QString idx = QString::number(i);
        m_wheelTelemetry["target_vel_" + idx] = msg.target_velocity[i];
        m_wheelTelemetry["current_vel_" + idx] = msg.current_velocity[i];
        m_wheelTelemetry["kp_" + idx] = msg.kp[i];
        m_wheelTelemetry["ki_" + idx] = msg.ki[i];
        m_wheelTelemetry["kd_" + idx] = msg.kd[i];
        m_wheelModels[i].addPoint(time, msg.target_velocity[i], msg.current_velocity[i]);
    }
    emit wheelTelemetryChanged();

    if (!m_isInitialized) {
        for (int i = 0; i < amr::NUM_WHEELS; ++i) {
            QString idx = QString::number(i);
            m_setWheelTelemetry["kp_" + idx]         = msg.kp[i];
            m_setWheelTelemetry["ki_" + idx]         = msg.ki[i];
            m_setWheelTelemetry["kd_" + idx]         = msg.kd[i];
            m_setWheelTelemetry["target_vel_" + idx] = msg.target_velocity[i];
        }
        m_isInitialized = true;
        emit isInitializedChanged();
        emit setWheelTelemetryChanged();
        emit initialGainsLoaded();
    }
}

void PidController::setPidGains(int wheelIndex, double kp, double ki, double kd)
{
    if (wheelIndex < 0 | wheelIndex >= amr::NUM_WHEELS) return;
    QString idx = QString::number(wheelIndex);
    m_setWheelTelemetry["kp_" + idx] = kp;
    m_setWheelTelemetry["ki_" + idx] = ki;
    m_setWheelTelemetry["kd_" + idx] = kd;

    emit setWheelTelemetryChanged();
    publishSetPoints();
}

void PidController::publishSetPoints() {
    amr_common::msg::WheelTelemetry msg;

    for (int i = 0; i < amr::NUM_WHEELS; ++i) {
        QString idx = QString::number(i);
        msg.kp[i]              = m_setWheelTelemetry.value("kp_" + idx).toDouble();
        msg.ki[i]              = m_setWheelTelemetry.value("ki_" + idx).toDouble();
        msg.kd[i]              = m_setWheelTelemetry.value("kd_" + idx).toDouble();
        msg.target_velocity[i] = m_setWheelTelemetry.value("target_vel_" + idx).toDouble();
        msg.current_velocity[i]= 0.0; // Current velocity is populated by hardware feedback
    }

    RosManager::instance().publishPidGains(msg);
}

void PidController::syncSetPoints() {
    std::cout << "Calling to syncSetPoints" << std::endl;
    for (int i = 0; i < amr::NUM_WHEELS; ++i) {
        QString idx = QString::number(i);
        m_setWheelTelemetry["kp_" + idx]         = m_wheelTelemetry["kp_" + idx];
        m_setWheelTelemetry["ki_" + idx]         = m_wheelTelemetry["ki_" + idx];
        m_setWheelTelemetry["kd_" + idx]         = m_wheelTelemetry["kd_" + idx];
        m_setWheelTelemetry["target_vel_" + idx] = m_wheelTelemetry["target_vel_" + idx];
    }
    emit setWheelTelemetryChanged();
    emit initialGainsLoaded();
}

// ----------------------------------------------------------------
// Getters
QVariantMap PidController::wheelTelemetry() const
{
    return m_wheelTelemetry;
}

QVariantMap PidController::setWheelTelemetry() const
{
    return m_setWheelTelemetry;
}

bool PidController::isInitialized() const
{
    return m_isInitialized;
}

// ----------------------------------------------------------------
// Setters

void PidController::setSetWheelTelemetry(const QVariantMap &map)
{
    if (m_setWheelTelemetry != map) {
        m_setWheelTelemetry = map;
        emit setWheelTelemetryChanged();
    }
}
