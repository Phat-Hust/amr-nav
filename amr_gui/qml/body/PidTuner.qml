// qml/pages/PidTuner.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../components"
import "../common"

ColumnLayout {
    id: tunnerRoot
    anchors.fill: parent
    spacing: 5

    RowLayout {
        Layout.fillWidth: true
        Layout.margins: 6
        spacing: 10
        Layout.preferredHeight: 40

        Item { Layout.fillWidth: true } // Spacer pushing button to the right

        CustomButton {
            text: "Sync from Robot"
            Layout.preferredWidth: 110
            textPixelSize: 11
            textBold: true
            textFontFamily: "Roboto"

            baseColor: AppColors.green
            hoverColor: AppColors.darkGreen
            textColor: AppColors.textBlack
            // Calls the Q_INVOKABLE method directly on click
            onClicked: {
                PidController.syncSetPoints()
            }
        }
    }

    GridLayout {
        columns: 2
        rows: 2

        WheelPlot {
            Layout.fillWidth: true
            Layout.fillHeight: true
            wheelName: "Front Left (Wheel 0)"
            model: PidController.wheelFL
            onApplyPid: (p, i, d) => PidController.setPidGains(0, p, i, d)
        }
        WheelPlot {
            Layout.fillWidth: true
            Layout.fillHeight: true
            wheelName: "Front Right (Wheel 1)"
            model: PidController.wheelFR
            onApplyPid: (p, i, d) => PidController.setPidGains(1, p, i, d)
        }
        WheelPlot {
            Layout.fillWidth: true
            Layout.fillHeight: true
            wheelName: "Rear Left (Wheel 2)"
            model: PidController.wheelRL
            onApplyPid: (p, i, d) => PidController.setPidGains(2, p, i, d)
        }
        WheelPlot {
            Layout.fillWidth: true
            Layout.fillHeight: true
            wheelName: "Rear Right (Wheel 3)"
            model: PidController.wheelRR
            onApplyPid: (p, i, d) => PidController.setPidGains(3, p, i, d)
        }
    }

    Component.onCompleted: {
        if (PidController.isInitialized) {
            PidController.syncSetPoints()
        }
    }

}
