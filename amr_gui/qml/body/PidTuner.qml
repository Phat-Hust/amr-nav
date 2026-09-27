// qml/pages/PidTuner.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../components"

GridLayout {
    columns: 2
    rows: 2

    WheelPlot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        wheelName: "Front Left (Wheel 0)"
        model: PidController.wheelFL
        onApplyPid: (p, i, d) => PidController.updateGains(0, p, i, d)
    }
    WheelPlot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        wheelName: "Front Right (Wheel 1)"
        model: PidController.wheelFR
        onApplyPid: (p, i, d) => PidController.updateGains(1, p, i, d)
    }
    WheelPlot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        wheelName: "Rear Left (Wheel 2)"
        model: PidController.wheelRL
        onApplyPid: (p, i, d) => PidController.updateGains(2, p, i, d)
    }
    WheelPlot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        wheelName: "Rear Right (Wheel 3)"
        model: PidController.wheelRR
        onApplyPid: (p, i, d) => PidController.updateGains(3, p, i, d)
    }
}
