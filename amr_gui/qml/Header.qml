// qml/Header.qml
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
    property color textColor: AppColors.textWhite
    property int activePage: 0

    signal pageSelected(int pageIndex)

    RowLayout {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        anchors.leftMargin: 16
        spacing: 12

        CustomButton {
            id: mapBtn
            text: "Map"
            Layout.fillHeight: true
            Layout.preferredWidth: 110

            textPixelSize: 14
            textBold: true
            textFontFamily: "Roboto"

            baseColor: headerRoot.activePage === 0 ? AppColors.green : AppColors.darkGreen
            hoverColor: AppColors.darkGreen
            textColor: AppColors.textWhite
            onClicked: headerRoot.pageSelected(0)
        }

        CustomButton {
            id: pidBtn
            text: "PID Debug"
            Layout.fillHeight: true
            Layout.preferredWidth: 110

            textPixelSize: 14
            textBold: true
            textFontFamily: "Roboto"
            baseColor: headerRoot.activePage === 1 ? AppColors.green : AppColors.darkGreen
            textColor: headerRoot.activePage === 1 ? "#000000" : AppColors.textWhite
            onClicked: headerRoot.pageSelected(1)
        }

        CustomButton {
            id: configBtn
            text: "Configuration"
            Layout.fillHeight: true
            Layout.preferredWidth: 110

            textPixelSize: 14
            textBold: true
            textFontFamily: "Roboto"
            baseColor: headerRoot.activePage === 1 ? AppColors.green : AppColors.darkGreen
            textColor: headerRoot.activePage === 1 ? "#000000" : AppColors.textWhite
            onClicked: headerRoot.pageSelected(1)
        }
    }

    Text {
        anchors.centerIn: parent
        text: headerRoot.tilteText
        color: headerRoot.textColor
        font.bold: true
        font.pixelSize: 16
    }
}
