import QtQuick 2.15
import QtQuick.Layouts 1.15
import "common"
import "components"

Rectangle {
    id: headerRoot
    height: 80
    color: AppColors.surfaceDark
    border.color: AppColors.borderDark
    border.width: 1
    property string tilteText: "AUTONOMOUS MOBILE ROBOT 01"
    property color textColor: AppColors.white
    property int activePage: 0

    signal pageSelected(int pageIndex)

    RowLayout {
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        CustomButton {
            text: "Map"
            implicitWidth: 100
            implicitHeight: 36
            baseColor: headerRoot.activePage === 0 ? AppColors.green : AppColors.darkGreen
            hoverColor: AppColors.darkGreen
            textColor: AppColors.textWhite
            onClicked: headerRoot.pageSelected(0)
        }

        CustomButton {
            text: "PID Debug"
            implicitWidth: 100
            implicitHeight: 36
            baseColor: headerRoot.activePage === 1 ? AppColors.primaryCyan : "#2c2c2c"
            textColor: headerRoot.activePage === 1 ? "#000000" : AppColors.textWhite
            onClicked: headerRoot.pageSelected(1)
        }
    }

    Text {
        anchors.centerIn: parent
        text: parent.tilteText
        color: parent.textColor
        font.bold: true
        font.pixelSize: 16
    }


}
