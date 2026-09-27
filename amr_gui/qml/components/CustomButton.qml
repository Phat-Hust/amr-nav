// qml/components/CustomButton.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import "../common"

Button {
    id: root

    property color baseColor: AppColors.buttonNormal
    property color hoverColor: AppColors.buttonHovered
    property color pressColor: AppColors.buttonPressed
    property color textColor: AppColors.textWhite

    property string iconSource: ""
    property int textPixelSize: 13
    property bool textBold: true
    property string textFontFamily: "sans-serif"

    implicitWidth: 120
    implicitHeight: 36

    contentItem: Item {
        anchors.fill: parent

        Row {
            anchors.centerIn: parent
            spacing: 6

            Image {
                id: btnIcon
                visible: root.iconSource !== ""
                source: root.iconSource
                width: 16
                height: 16
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
            }

            Text {
                text: root.text
                color: root.textColor
                font.pixelSize: root.textPixelSize
                font.bold: root.textBold
                font.family: root.textFontFamily
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    background: Rectangle {
        anchors.fill: parent
        radius: 4
        border.color: AppColors.borderDark
        border.width: 1

        color: {
            if (!root.enabled) return "#444444";
            if (root.down) return root.pressColor;
            if (root.hovered) return root.hoverColor;
            return root.baseColor;
        }

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }
}
