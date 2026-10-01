import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../common"

Rectangle {
    id: root

    property int wheelIndex: 0
    property string wheelName: ""
    property var model: null
    signal applyPid(double kp, double ki, double kd)

    border.color: "#3d3d3d"
    border.width: 1
    color: AppColors.white

    // Helper function to extract telemetry values and set the SpinBoxes
    function syncFromTelemetry() {
        var idx = root.wheelIndex.toString()
        var p = PidController.setWheelTelemetry["kp_" + idx]
        var i = PidController.setWheelTelemetry["ki_" + idx]
        var d = PidController.setWheelTelemetry["kd_" + idx]

        if (p !== undefined && p !== null) kpBox.value = Math.round(p * 100.0)
        if (i !== undefined && i !== null) kiBox.value = Math.round(i * 100.0)
        if (d !== undefined && d !== null) kdBox.value = Math.round(d * 100.0)
    }

    // 1. Listen for the initial/manual sync signal from C++
    Connections {
        target: PidController
        function onInitialGainsLoaded() {
            root.syncFromTelemetry()
        }
    }

    // 2. If ROS data arrived before this QML page loaded, populate immediately
    Component.onCompleted: {
        if (PidController.isInitialized) {
            root.syncFromTelemetry()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Text {
            text: root.wheelName
            color: AppColors.deepSkyBlue
            font.bold: true
            font.pixelSize: 15
        }

        Canvas {
            id: plotCanvas
            Layout.fillWidth: true
            Layout.fillHeight: true

            property double maxSpeed: 1.0

            readonly property real marginLeft: 60
            readonly property real marginRight: 15
            readonly property real marginTop: 15
            readonly property real marginBottom: 25

            Connections {
                target: root.model
                function onPointsChanged() {
                    plotCanvas.requestPaint();
                }
            }

            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);

                var plotW = width - marginLeft - marginRight;
                var plotH = height - marginTop - marginBottom;
                var zeroY = marginTop + (plotH / 2.0);

                // 1. Grid Lines & Labels
                ctx.font = "10px monospace";
                ctx.textAlign = "right";
                ctx.textBaseline = "middle";

                var numDivisions = 6;
                for (var step = 0; step <= numDivisions; ++step) {
                    var ratio = step / numDivisions;
                    var curY = marginTop + (ratio * plotH);
                    var speedVal = maxSpeed - (ratio * (2.0 * maxSpeed));

                    ctx.strokeStyle = (Math.abs(speedVal) < 0.01) ? "#555555" : "#222222";
                    ctx.lineWidth = (Math.abs(speedVal) < 0.01) ? 1.5 : 1.0;
                    ctx.beginPath();
                    ctx.moveTo(marginLeft, curY);
                    ctx.lineTo(width - marginRight, curY);
                    ctx.stroke();

                    ctx.fillStyle = (Math.abs(speedVal) < 0.01) ? "#FFFFFF" : "#777777";
                    ctx.fillText(speedVal.toFixed(1) + " m/s", marginLeft - 6, curY);
                }

                // 2. Axes
                ctx.strokeStyle = AppColors.black;
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(marginLeft, marginTop);
                ctx.lineTo(marginLeft, height - marginBottom);
                ctx.moveTo(marginLeft, height - marginBottom);
                ctx.lineTo(width - marginRight, height - marginBottom);
                ctx.stroke();

                // 3. Sliding lines
                function drawSlidingLine(points, strokeColor) {
                    if (!points || points.length < 2) return;

                    var count = points.length;
                    ctx.strokeStyle = strokeColor;
                    ctx.lineWidth = 2;
                    ctx.beginPath();

                    for (var i = 0; i < count; ++i) {
                        var pt = points[i];
                        var val = 0.0;
                        if (pt.y !== undefined) {
                            val = (typeof pt.y === "function") ? pt.y() : pt.y;
                        }

                        var x = marginLeft + ((i / Math.max(count - 1, 1)) * plotW);
                        var normalizedY = val / plotCanvas.maxSpeed;
                        var y = zeroY - (normalizedY * (plotH / 2.0));

                        if (i === 0) {
                            ctx.moveTo(x, y);
                        } else {
                            ctx.lineTo(x, y);
                        }
                    }
                    ctx.stroke();
                }

                if (root.model) {
                    drawSlidingLine(root.model.targetPoints, AppColors.accentRed);
                    drawSlidingLine(root.model.currentPoints, AppColors.deepSkyBlue);
                }

                // 4. Legend
                var legX = width - marginRight - 160;
                var legY = marginTop + 10;

                ctx.strokeStyle = AppColors.accentRed;
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(legX, legY);
                ctx.lineTo(legX + 16, legY);
                ctx.stroke();

                ctx.fillStyle = AppColors.black;
                ctx.textAlign = "left";
                ctx.textBaseline = "middle";
                ctx.font = "bold 11px sans-serif";
                ctx.fillText("Target", legX + 22, legY);

                ctx.strokeStyle = AppColors.deepSkyBlue;
                ctx.beginPath();
                ctx.moveTo(legX + 80, legY);
                ctx.lineTo(legX + 96, legY);
                ctx.stroke();

                ctx.fillText("Current", legX + 102, legY);
            }
        }

        // PID Controls
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

                // Kp
                Text { text: "Kp:"; color: "#ffffff"; font.bold: true; font.pixelSize: 13 }
                SpinBox {
                    id: kpBox
                    from: 0; to: 10000; value: 100; stepSize: 5
                    editable: true
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 34
                    textFromValue: function(value, locale) { return (value / 100.0).toFixed(2); }
                    valueFromText: function(text, locale) { return Math.round(parseFloat(text) * 100); }
                    contentItem: TextInput {
                        text: kpBox.textFromValue(kpBox.value, kpBox.locale)
                        font: kpBox.font
                        color: "#ffffff"
                        selectionColor: "#007acc"
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !kpBox.editable
                        validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                    background: Rectangle {
                        color: "#333333"
                        border.color: kpBox.activeFocus ? "#00e5ff" : "#555555"
                        radius: 4
                    }
                }

                // Ki
                Text { text: "Ki:"; color: "#ffffff"; font.bold: true; font.pixelSize: 13 }
                SpinBox {
                    id: kiBox
                    from: 0; to: 10000; value: 10; stepSize: 1
                    editable: true
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 34
                    textFromValue: function(value, locale) { return (value / 100.0).toFixed(2); }
                    valueFromText: function(text, locale) { return Math.round(parseFloat(text) * 100); }
                    contentItem: TextInput {
                        text: kiBox.textFromValue(kiBox.value, kiBox.locale)
                        font: kiBox.font
                        color: "#ffffff"
                        selectionColor: "#007acc"
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !kiBox.editable
                        validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                    background: Rectangle {
                        color: "#333333"
                        border.color: kiBox.activeFocus ? "#00e5ff" : "#555555"
                        radius: 4
                    }
                }

                // Kd
                Text { text: "Kd:"; color: "#ffffff"; font.bold: true; font.pixelSize: 13 }
                SpinBox {
                    id: kdBox
                    from: 0; to: 10000; value: 5; stepSize: 1
                    editable: true
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 34
                    textFromValue: function(value, locale) { return (value / 100.0).toFixed(2); }
                    valueFromText: function(text, locale) { return Math.round(parseFloat(text) * 100); }
                    contentItem: TextInput {
                        text: kdBox.textFromValue(kdBox.value, kdBox.locale)
                        font: kdBox.font
                        color: "#ffffff"
                        selectionColor: "#007acc"
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !kdBox.editable
                        validator: DoubleValidator { bottom: 0.0; top: 100.0; decimals: 2 }
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                    background: Rectangle {
                        color: "#333333"
                        border.color: kdBox.activeFocus ? "#00e5ff" : "#555555"
                        radius: 4
                    }
                }

                Item { Layout.fillWidth: true }

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
