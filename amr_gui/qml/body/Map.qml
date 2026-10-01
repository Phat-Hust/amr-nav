import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import "../common"
import "../components"

Rectangle {
    id: mapPageRoot
    anchors.fill: parent
    color: AppColors.antiqueWhite

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // LEFT COLUMN: High Performance C++ Map & Lidar View
        Rectangle {
            id: mapContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: AppColors.white
            radius: 8
            border.color: AppColors.antiqueWhite
            border.width: 1
            clip: true

//            MapViewItem {
//                id: mapView
//                anchors.fill: parent

//                MouseArea {
//                    anchors.fill: parent
//                    hoverEnabled: true
//                    acceptedButtons: Qt.LeftButton | Qt.RightButton

//                    property real lastX: 0
//                    property real lastY: 0

//                    onPressed: (mouse) => {
//                        lastX = mouse.x
//                        lastY = mouse.y
//                    }

//                    onPositionChanged: (mouse) => {
//                        if (pressedButtons & Qt.LeftButton) {
//                            var dx = mouse.x - lastX
//                            var dy = mouse.y - lastY
//                            mapView.addPan(dx, dy)
//                            lastX = mouse.x
//                            lastY = mouse.y
//                        }
//                    }

//                    // Mouse wheel to zoom
//                    onWheel: (wheel) => {
//                        var factor = (wheel.angleDelta.y > 0) ? 1.15 : 0.85
//                        mapView.zoomAt(factor, Qt.point(wheel.x, wheel.y))
//                    }
//                }
//            }

            // Quick reset zoom/pan overlay button
            CustomButton {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 12
                text: "Reset View"
                Layout.preferredWidth: 90
                Layout.preferredHeight: 30
                textPixelSize: 11
                onClicked: mapView.resetView()
            }
        }

        // RIGHT COLUMN: Management Panel & Dynamic Pose Readout
        Rectangle {
            id: controlPanel
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            color: AppColors.backgroundLight
            radius: 8
            border.color: AppColors.borderLight
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                Text {
                    text: "Map & Navigation"
                    color: "#ffffff"
                    font.bold: true
                    font.pixelSize: 16
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#383838"
                }

                CustomButton {
                    text: "Load Map"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    textPixelSize: 13
                    textBold: true
                    baseColor: AppColors.green
                    hoverColor: AppColors.darkGreen
                    textColor: AppColors.textBlack
                    onClicked: mapFileDialog.open()
                }

                Text {
                    text: "Robot Pose (Map Frame)"
                    color: "#bbbbbb"
                    font.bold: true
                    font.pixelSize: 13
                    Layout.topMargin: 10
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    color: "#181818"
                    radius: 6
                    border.color: "#2c2c2c"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "X Axis:"; color: "#888888"; font.pixelSize: 13; Layout.preferredWidth: 70 }
                            Text {
                                text: mapView.robotX.toFixed(3) + " m"
                                color: "#00e5ff"
                                font.bold: true
                                font.family: "Monospace"
                                font.pixelSize: 14
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "Y Axis:"; color: "#888888"; font.pixelSize: 13; Layout.preferredWidth: 70 }
                            Text {
                                text: mapView.robotY.toFixed(3) + " m"
                                color: "#00e5ff"
                                font.bold: true
                                font.family: "Monospace"
                                font.pixelSize: 14
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "Yaw:"; color: "#888888"; font.pixelSize: 13; Layout.preferredWidth: 70 }
                            Text {
                                text: (mapView.robotYaw * (180.0 / Math.PI)).toFixed(1) + " °"
                                color: "#ffd54f"
                                font.bold: true
                                font.family: "Monospace"
                                font.pixelSize: 14
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }

    FileDialog {
        id: mapFileDialog
        title: "Select Map File (.yaml / .pbstream)"
        nameFilters: ["Map files (*.yaml *.pbstream)", "All files (*)"]

        onAccepted: {
            // In Qt 6, selectedFile returns a QUrl directly
            var path = mapFileDialog.selectedFile.toString().replace(/^(file:\/{2})/,"");
            var filename = path.substring(path.lastIndexOf('/') + 1);
            mapPageRoot.currentMapName = filename;
            console.log("Loading selected map from:", path);
        }
    }
}
