// qml/Footer.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import "common"
import "components"

Rectangle {
    id: footerRoot
    height: 48
    color: AppColors.backgroundDark
    border.color: AppColors.borderDark
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 12

        // Status Indicator
        Text {
            text: "ROS Status: Connected"
            color: AppColors.primaryCyan
            font.bold: true
            font.pixelSize: 13
            Layout.alignment: Qt.AlignVCenter
        }

        // Spacer pushes buttons to the right
        Item { Layout.fillWidth: true }

        // Reusable Buttons
        CustomButton {
            text: "Reset Plots"
            baseColor: "#333333"
            hoverColor: "#444444"
            pressColor: "#222222"
            onClicked: {
                console.log("Reset Plots triggered");
            }
        }

        CustomButton {
            text: "Sync Gains"
            baseColor: AppColors.buttonNormal
            hoverColor: AppColors.buttonHovered
            pressColor: AppColors.buttonPressed
            textColor: AppColors.textDimmed
            onClicked: {
                console.log("Sync Gains clicked");
            }
        }

        CustomButton {
            text: "Emergency Stop"
            baseColor: "#b71c1c"
            hoverColor: "#d32f2f"
            pressColor: "#8e0000"
            onClicked: {
                console.log("EMERGENCY STOP TRIGGERED");
            }
        }
    }
}
