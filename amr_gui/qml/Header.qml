import QtQuick 2.15
import QtQuick.Layouts 1.15
import "common"

Rectangle {
    height: 80
    color: AppColors.surfaceDark
    border.color: AppColors.borderDark
    border.width: 1

    Text {
        anchors.centerIn: parent
        text: "AUTONOMOUS MOBILE ROBOT"
        color: AppColors.textWhite
        font.bold: true
        font.pixelSize: 16
    }
}
