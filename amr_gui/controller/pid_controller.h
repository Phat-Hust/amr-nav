// controller/pid_controller.h
#pragma once
#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QElapsedTimer>
#include "../common/defines.h"
#include "../common/ros_manager.h"
#include "../model/wheel_data_model.h"

class PidController : public QObject {
    Q_OBJECT
    // Feedback from ros (ros_manager -> Controller)
    Q_PROPERTY(WheelDataModel* wheelFL READ wheelFL CONSTANT)
    Q_PROPERTY(WheelDataModel* wheelFR READ wheelFR CONSTANT)
    Q_PROPERTY(WheelDataModel* wheelRL READ wheelRL CONSTANT)
    Q_PROPERTY(WheelDataModel* wheelRR READ wheelRR CONSTANT)
    Q_PROPERTY(QVariantMap wheelTelemetry READ wheelTelemetry NOTIFY wheelTelemetryChanged)
    Q_PROPERTY(QVariantMap setWheelTelemetry READ setWheelTelemetry WRITE setSetWheelTelemetry NOTIFY setWheelTelemetryChanged)
    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY isInitializedChanged)
public:
    explicit PidController(QObject *parent = nullptr);
    WheelDataModel* wheelFL() { return &m_wheelModels[amr::FRONT_LEFT]; }
    WheelDataModel* wheelFR() { return &m_wheelModels[amr::FRONT_RIGHT]; }
    WheelDataModel* wheelRL() { return &m_wheelModels[amr::REAR_LEFT]; }
    WheelDataModel* wheelRR() { return &m_wheelModels[amr::REAR_RIGHT]; }


    // Getters for Q_PROPERTY
    QVariantMap wheelTelemetry() const;
    QVariantMap setWheelTelemetry() const;
    bool isInitialized() const;

    // Setters for Q_PROPERTY
    void setSetWheelTelemetry(const QVariantMap &map);

    // Q_INVOKABLE methods

    Q_INVOKABLE void setPidGains(int wheelIndex, double kp, double ki, double kd);
    Q_INVOKABLE void publishSetPoints();
    Q_INVOKABLE void syncSetPoints();

public slots:
    void updateWheelTelemetry(const amr_common::msg::WheelTelemetry& msg);

signals:
    void wheelTelemetryChanged();
    void setWheelTelemetryChanged();
    void isInitializedChanged();
    void initialGainsLoaded();

private:
    std::array<WheelDataModel, amr::NUM_WHEELS> m_wheelModels;
    QElapsedTimer m_timer;
    QVariantMap m_wheelTelemetry;
    QVariantMap m_setWheelTelemetry;
    bool m_isInitialized{false};
};
