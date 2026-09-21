import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "common"
import "components"
import "body"


ApplicationWindow {
    id: appWindow
    visible: true
    width: 1280
    height: 800
    title: "AMR Mission & Tuning Dashboard"
    color: AppColors.antiqueWhite

    property int currentPage: 0
    readonly property var pageTitles: ["Map View", "PID Debug View"]

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Header {
            Layout.fillWidth: true
            color: AppColors.deepSkyBlue
            tilteText: "AUTONOMOUS MOBILE ROBOT 01"
            textColor: AppColors.white
            activePage: appWindow.currentPage
            onPageSelected: (index) => {
                appWindow.currentPage = index;
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: appWindow.currentPage

            Map {}
            PidTuner {}
        }

        Footer {
            Layout.fillWidth: true
            color: AppColors.deepSkyBlue
            currentPageName: appWindow.pageTitles[appWindow.currentPage]
        }
    }
}
