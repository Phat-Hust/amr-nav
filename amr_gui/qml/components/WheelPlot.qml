import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../common"

Rectangle {
    id: root
    property string wheelName: ""
    property var model: null
    signal applyPid(double kp, double ki, double kd)

    border.color: "#3d3d3d"
    border.width: 1
    color: "#181818"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Text {
            text: root.wheelName
            color: "#00e5ff"
            font.bold: true
            font.pixelSize: 15
        }

        Canvas {
            id: plotCanvas
            Layout.fillWidth: true
            Layout.fillHeight: true

            Connections {
                target: root.model
                function onPointsChanged() { plotCanvas.requestPaint(); }
            }

            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);

                ctx.strokeStyle = "#282828";
                ctx.lineWidth = 1;
                ctx.beginPath();
                for (var y = 0; y < height; y += 40) {
                    ctx.moveTo(0, y);
                    ctx.lineTo(width, y);
                }
                ctx.stroke();

                function drawLine(points, strokeStyle) {
                    if (!points || points.length === 0) return;
                    ctx.strokeStyle = strokeStyle;
                    ctx.lineWidth = 2;
                    ctx.beginPath();
                    for (var i = 0; i < points.length; i++) {
                        var x = (i / 200) * width;
                        var y = height - (points[i].y * (height / 2) + (height / 2));
                        if (i === 0) ctx.moveTo(x, y);
                        else ctx.lineTo(x, y);
                    }
                    ctx.stroke();
                }

                drawLine(root.model ? root.model.targetPoints : [], "#00e5ff"); // Target
                drawLine(root.model ? root.model.currentPoints : [], "#ff3366"); // Current
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 52
            color: "#242424"
            radius: 6
            border.color: "#383838"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                // Kp input
                Text { text: "Kp:"; color: "#ffffff"; font.bold: true; font.pixelSize: 13 }
                SpinBox {
                    id: kpBox
                    from: 0; to: 10000; value: 100; stepSize: 5
                    editable: true
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 34
                    contentItem: TextInput {
                        text: kpBox.textFromValue(kpBox.value, kpBox.locale)
                        font: kpBox.font
                        color: "#ffffff"
                        selectionColor: "#007acc"
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !kpBox.editable
                        validator: kpBox.validator
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                    background: Rectangle {
                        color: "#333333"
                        border.color: kpBox.activeFocus ? "#00e5ff" : "#555555"
                        radius: 4
                    }
                }

                // Ki input
                Text { text: "Ki:"; color: "#ffffff"; font.bold: true; font.pixelSize: 13 }
                SpinBox {
                    id: kiBox
                    from: 0; to: 10000; value: 10; stepSize: 1
                    editable: true
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 34
                    contentItem: TextInput {
                        text: kiBox.textFromValue(kiBox.value, kiBox.locale)
                        font: kiBox.font
                        color: "#ffffff"
                        selectionColor: "#007acc"
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !kiBox.editable
                        validator: kiBox.validator
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                    background: Rectangle {
                        color: "#333333"
                        border.color: kiBox.activeFocus ? "#00e5ff" : "#555555"
                        radius: 4
                    }
                }

                // Kd input
                Text { text: "Kd:"; color: "#ffffff"; font.bold: true; font.pixelSize: 13 }
                SpinBox {
                    id: kdBox
                    from: 0; to: 10000; value: 5; stepSize: 1
                    editable: true
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 34
                    contentItem: TextInput {
                        text: kdBox.textFromValue(kdBox.value, kdBox.locale)
                        font: kdBox.font
                        color: "#ffffff"
                        selectionColor: "#007acc"
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !kdBox.editable
                        validator: kdBox.validator
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                    background: Rectangle {
                        color: "#333333"
                        border.color: kdBox.activeFocus ? "#00e5ff" : "#555555"
                        radius: 4
                    }
                }

                Item { Layout.fillWidth: true } // Khoảng trống giãn cách

                Button {
                    id: updateBtn
                    text: "Set PID"
                    Layout.preferredWidth: 85
                    Layout.preferredHeight: 34
                    contentItem: Text {
                        text: updateBtn.text
                        font.bold: true
                        font.pixelSize: 13
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: updateBtn.down ? "#0088a8" : (updateBtn.hovered ? "#00b8d4" : "#0097a7")
                        radius: 4
                    }
                    onClicked: root.applyPid(kpBox.value / 100.0, kiBox.value / 100.0, kdBox.value / 100.0)
                }
            }
        }
    }
}
