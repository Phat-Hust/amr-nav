#pragma once
#include <QString>

namespace amr {
    constexpr int NUM_WHEELS = 4;
    constexpr int MAX_BUFFER_POINTS = 200;

    const QString TOPIC_WHEEL_TELEMETRY = "/wheel_telemetry";
    const QString TOPIC_SET_PID         = "/set_wheel_pid";
    const QString TOPIC_EMERGENCY_BUTTON = "/emergency_btn";

    enum WheelIndex {
        FRONT_LEFT  = 0,
        FRONT_RIGHT = 1,
        REAR_LEFT   = 2,
        REAR_RIGHT  = 3
    };
}
