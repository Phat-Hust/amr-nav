import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "common"
import "components"


ApplicationWindow {
    id: appWindow
    visible: true
    width: 1280
    height: 800
    title: "AMR Mission & Tuning Dashboard"
    color: AppColors.backgroundDark

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Header {
            Layout.fillWidth: true
            color: AppColors.backgroundLight
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
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

        Footer {
            Layout.fillWidth: true
            color: AppColors.backgroundLight
        }
    }
}
