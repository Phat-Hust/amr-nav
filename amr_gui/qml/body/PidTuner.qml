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
        model: PidCtrl.wheelFL
        onApplyPid: (p, i, d) => PidCtrl.updateGains(0, p, i, d)
    }
    WheelPlot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        wheelName: "Front Right (Wheel 1)"
        model: PidCtrl.wheelFR
        onApplyPid: (p, i, d) => PidCtrl.updateGains(1, p, i, d)
    }
    WheelPlot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        wheelName: "Rear Left (Wheel 2)"
        model: PidCtrl.wheelRL
        onApplyPid: (p, i, d) => PidCtrl.updateGains(2, p, i, d)
    }
    WheelPlot {
        Layout.fillWidth: true
        Layout.fillHeight: true
        wheelName: "Rear Right (Wheel 3)"
        model: PidCtrl.wheelRR
        onApplyPid: (p, i, d) => PidCtrl.updateGains(3, p, i, d)
    }
}
