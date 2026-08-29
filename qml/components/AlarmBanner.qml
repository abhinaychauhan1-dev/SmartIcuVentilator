import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property var dashboardViewModel

    readonly property int priority: dashboardViewModel
                                            ? dashboardViewModel.highestAlarmPriority : 0
    readonly property int silenceRemaining: dashboardViewModel
                                                   ? dashboardViewModel.silenceSecondsRemaining : 0
    property real highFlashOpacity: 1.0
    property real silenceProgress: Math.max(0.0, Math.min(1.0,
                                                         silenceRemaining / 120.0))

    implicitWidth: 960
    implicitHeight: 76
    color: "#20282D"

    Behavior on silenceProgress {
        NumberAnimation { duration: 350; easing.type: Easing.OutCubic }
    }

    SequentialAnimation {
        running: root.priority === 3
        loops: Animation.Infinite
        NumberAnimation { target: root; property: "highFlashOpacity"; from: 1.0; to: 0.35; duration: 250; easing.type: Easing.InOutQuad }
        NumberAnimation { target: root; property: "highFlashOpacity"; from: 0.35; to: 1.0; duration: 250; easing.type: Easing.InOutQuad }
    }

    Rectangle {
        anchors.fill: parent
        color: root.priority === 3 ? "#FF1744"
               : root.priority === 2 ? "#FF9100"
               : root.priority === 1 ? "#00E5FF"
               : "#20282D"
        opacity: root.priority === 3 ? root.highFlashOpacity : 1.0

        Behavior on color { ColorAnimation { duration: 120 } }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 12
        spacing: 16

        Rectangle {
            Layout.preferredWidth: 10
            Layout.preferredHeight: 48
            color: root.priority === 0 ? "#64747C" : root.priority === 1 ? "#003C45" : "#FFFFFF"
            radius: 2
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1

            Text {
                text: root.priority === 3 ? "HIGH PRIORITY"
                      : root.priority === 2 ? "MEDIUM PRIORITY"
                      : root.priority === 1 ? "LOW PRIORITY"
                      : "VENTILATOR STATUS"
                color: root.priority === 1 ? "#001A1E" : "#FFFFFF"
                font.family: "Bahnschrift"
                font.pixelSize: 12
                font.weight: Font.Bold
            }

            Text {
                text: root.priority === 0
                      ? "SYSTEM NORMAL"
                      : root.dashboardViewModel.activeAlarmText
                color: root.priority === 1 ? "#001A1E" : "#FFFFFF"
                font.family: "Bahnschrift"
                font.pixelSize: 22
                font.weight: Font.Bold
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        Button {
            id: silenceButton

            Layout.preferredWidth: 214
            Layout.minimumHeight: 56
            text: root.silenceRemaining > 0
                  ? "AUDIO SILENCED  " + root.silenceRemaining + "s"
                  : "120s Audio Silence"
            enabled: root.priority > 0
            hoverEnabled: true
            focusPolicy: Qt.StrongFocus
            Accessible.name: text
            onClicked: root.dashboardViewModel.silenceAudioAlarm(120)

            background: Rectangle {
                  color: !silenceButton.enabled ? "#293238"
                      : silenceButton.down ? "#243239" : "#11191E"
                  border.color: !silenceButton.enabled ? "#56636A"
                          : silenceButton.activeFocus ? "#FFFFFF" : "#B6C5CB"
                border.width: silenceButton.activeFocus ? 3 : 1
                radius: 6
            }

            contentItem: RowLayout {
                spacing: 9

                Item {
                    Layout.preferredWidth: 38
                    Layout.preferredHeight: 38

                    Canvas {
                        id: silenceProgressCanvas

                        anchors.fill: parent
                        antialiasing: true
                        onPaint: {
                            const context = getContext("2d")
                            const center = width / 2
                            const radius = Math.min(width, height) / 2 - 3
                            context.reset()
                            context.lineWidth = 4
                            context.lineCap = "round"
                            context.strokeStyle = "#45565E"
                            context.beginPath()
                            context.arc(center, center, radius, 0, Math.PI * 2)
                            context.stroke()

                            if (root.silenceProgress > 0) {
                                context.strokeStyle = "#00E5FF"
                                context.beginPath()
                                context.arc(center, center, radius, -Math.PI / 2,
                                            -Math.PI / 2 + Math.PI * 2
                                            * root.silenceProgress)
                                context.stroke()
                            }
                        }

                        Connections {
                            target: root
                            function onSilenceProgressChanged() {
                                silenceProgressCanvas.requestPaint()
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: root.silenceRemaining > 0 ? Math.ceil(root.silenceRemaining / 10) : "♪"
                        color: "#EAF4F7"
                        font.family: "Bahnschrift"
                        font.pixelSize: root.silenceRemaining > 0 ? 11 : 18
                        font.weight: Font.Bold
                    }
                }

                Text {
                    text: silenceButton.text
                    color: silenceButton.enabled ? "#FFFFFF" : "#87949A"
                    font.family: "Bahnschrift"
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                }
            }
        }
    }
}