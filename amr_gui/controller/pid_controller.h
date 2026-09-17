// controller/pid_controller.h
#pragma once
#include <QObject>
#include <QElapsedTimer>
#include "../common/defines.h"
#include "../common/ros_manager.h"
#include "../model/wheel_data_model.h"

class PidController : public QObject {
    Q_OBJECT
    Q_PROPERTY(WheelDataModel* wheelFL READ wheelFL CONSTANT)
    Q_PROPERTY(WheelDataModel* wheelFR READ wheelFR CONSTANT)
    Q_PROPERTY(WheelDataModel* wheelRL READ wheelRL CONSTANT)
    Q_PROPERTY(WheelDataModel* wheelRR READ wheelRR CONSTANT)

public:
    explicit PidController(QObject *parent = nullptr);

    WheelDataModel* wheelFL() { return &m_wheels[amr::FRONT_LEFT]; }
    WheelDataModel* wheelFR() { return &m_wheels[amr::FRONT_RIGHT]; }
    WheelDataModel* wheelRL() { return &m_wheels[amr::REAR_LEFT]; }
    WheelDataModel* wheelRR() { return &m_wheels[amr::REAR_RIGHT]; }

    Q_INVOKABLE void updateGains(int wheelIndex, double kp, double ki, double kd);

private slots:
    void handleTelemetry(const amr_common::msg::WheelTelemetry::SharedPtr msg);

private:
    WheelDataModel m_wheels[amr::NUM_WHEELS];
    QElapsedTimer m_timer;
};
